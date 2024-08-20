#ifndef UI_IMGUI_TEXT_EDIT_H
#define UI_IMGUI_TEXT_EDIT_H

#include <imgui/TextEditor.h>

#include <string>

namespace ui
{

class TextEditLayer
{
public:
	TextEditLayer() = default;
	~TextEditLayer() = default;
	void init();
	void open(const char* file_path);
	void quick_save(const std::string& str);
	void save(const std::string& str);
	void draw(bool* p_open);
	void draw_to_modal(const char* path);
	const char* get_modal_name();

private:
	TextEditor editor_;
	TextEditor::LanguageDefinition lang_ = TextEditor::LanguageDefinition::CPlusPlus();
	const char* ppnames_[1] = {"NULL"};
	const char* ppvalues_[1] = {"#define NULL ((void*)0)"};	   // set your own identifiers
	const char* identifiers_[1] = {"HWND"};
	const char* idecls_[1] = {"ssss"};
	std::string current_file_name_ = "";
	std::string modal_name_ = "editor";
	std::string current_file_path_ = "";
	bool is_save_open_ = false;
};
}	 // namespace ui

#endif