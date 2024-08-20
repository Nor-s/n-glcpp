#include "text_edit_layer.h"

#include "imgui/imgui.h"
#include <filesystem>
#include <fstream>

namespace ui
{
void TextEditLayer::init()
{
	for (int i = 0; i < static_cast<int>(sizeof(ppnames_) / sizeof(ppnames_[0])); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues_[i];
		lang_.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames_[i]), id));
	}

	for (int i = 0; i < static_cast<int>(sizeof(identifiers_) / sizeof(identifiers_[0])); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls_[i]);
		lang_.mIdentifiers.insert(std::make_pair(std::string(identifiers_[i]), id));
	}
	editor_.SetLanguageDefinition(lang_);
}
void TextEditLayer::open(const char* file_path)
{
	auto path = std::filesystem::path(file_path);
	if (current_file_name_ == path.filename().string())
	{
		return;
	}
	std::ifstream file_stream(file_path);
	if (file_stream.good())
	{
		std::string str((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
		editor_.SetText(str);
		current_file_name_ = path.filename().string();
		current_file_path_ = path.string();
		lang_.mName = path.extension().string();
		editor_.SetLanguageDefinition(lang_);
	}
	if (file_stream.is_open())
	{
		file_stream.close();
	}
}
void TextEditLayer::quick_save(const std::string& str)
{
	std::string path = "./tmp.json";
	if (current_file_path_ != "")
	{
		path = current_file_path_;
	}
	std::ofstream file_stream(path.c_str());
	if (file_stream.good())
	{
		file_stream.write(str.c_str(), str.size());
	}
	if (file_stream.is_open())
	{
		file_stream.close();
	}
}
void TextEditLayer::save(const std::string& str)
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Save?", &is_save_open_, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("save", ImVec2(120, 0)))
		{
			quick_save(str);
			is_save_open_ = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			is_save_open_ = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void TextEditLayer::draw(bool* p_open)
{
	if (*p_open)
	{
		ImGuiWindowFlags window_flags = 0;
		window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
		auto cpos = editor_.GetCursorPosition();
		ImGui::Begin("Text Editor Demo", p_open, window_flags);
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					is_save_open_ = true;
				}

				if (ImGui::MenuItem("Quit"))
				{
					*p_open = false;
				}
				// if (ImGui::MenuItem("Open"))
				// {
				//     nfdchar_t *outPath;
				//     nfdfilteritem_t filterItem[1] = {{"file", "glsl,json"}};
				//     nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);

				//     if (result == NFD_OKAY)
				//     {
				//         open(outPath);
				//         NFD_FreePath(outPath);
				//     }
				//     else if (result == NFD_CANCEL)
				//     {
				//     }
				//     else
				//     {
				//     }

				//     NFD_Quit();
				// }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = editor_.IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					editor_.SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor_.CanUndo()))
					editor_.Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor_.CanRedo()))
					editor_.Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor_.HasSelection()))
					editor_.Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor_.HasSelection()))
					editor_.Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor_.HasSelection()))
					editor_.Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					editor_.Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					editor_.SetSelection(TextEditor::Coordinates(),
										 TextEditor::Coordinates(editor_.GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark palette"))
					editor_.SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					editor_.SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					editor_.SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		if (ImGui::SmallButton("save"))
		{
			quick_save(editor_.GetText());
		}
		ImGui::SameLine();
		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1,
					editor_.GetTotalLines(), editor_.IsOverwrite() ? "Ovr" : "Ins", editor_.CanUndo() ? "*" : " ",
					editor_.GetLanguageDefinition().mName.c_str(), current_file_name_.c_str());

		editor_.Render("TextEditor");

		if (is_save_open_)
		{
			if (!ImGui::IsPopupOpen("Save?"))
				ImGui::OpenPopup("Save?");
			save(editor_.GetText());
		}

		ImGui::End();
	}
}
void TextEditLayer::draw_to_modal(const char* path)
{
	open(path);

	// ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	// ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	auto cpos = editor_.GetCursorPosition();
	if (ImGui::BeginPopupModal(modal_name_.c_str(), NULL,
							   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))

	{
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
		ImGui::Separator();

		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1,
					editor_.GetTotalLines(), editor_.IsOverwrite() ? "Ovr" : "Ins", editor_.CanUndo() ? "*" : " ",
					editor_.GetLanguageDefinition().mName.c_str(), current_file_name_.c_str());

		editor_.Render("TextEditor");
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
const char* TextEditLayer::get_modal_name()
{
	return modal_name_.c_str();
}
}	 // namespace ui