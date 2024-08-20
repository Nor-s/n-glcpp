#include "hierarchy_layer.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "../entity/entity.h"
#include "../scene/scene.hpp"
#include <imgui/icons/icons.h>
#include <entity/components/pose_component.h>
#include <entity/components/renderable/armature_component.h>
#include <entity/components/renderable/mesh_component.h>
namespace ui
{
HierarchyLayer::HierarchyLayer() = default;
HierarchyLayer::~HierarchyLayer() = default;
void HierarchyLayer::draw(Scene* scene, UiContext& ui_context)
{
	ImGuiTreeNodeFlags node_flags =
		ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	auto& entities = scene->get_mutable_ref_shared_resources()->get_mutable_entities();
	auto selected_entity = scene->get_mutable_selected_entity();
	selected_id_ = -2;
	if (selected_entity)
	{
		selected_id_ = selected_entity->get_id();
	}
	ImGui::Begin("Hierarchy");
	{
		if (entities)
		{
			auto selected_node = draw_tree_node(entities.get(), node_flags);
			if (selected_node && !(selected_entity && selected_entity->get_id() == selected_node->get_id()))
			{
				ui_context.entity.is_changed_selected_entity = true;
				ui_context.entity.selected_id = selected_node->get_id();
			}
		}
	}
	ImGui::End();
}

anim::Entity* HierarchyLayer::draw_tree_node(anim::Entity* entity_node, const ImGuiTreeNodeFlags& node_flags, int depth)
{
	auto selected_flags = node_flags;
	anim::Entity* selected_entity = nullptr;
	auto& childrens = entity_node->get_mutable_children();
	if (selected_id_ == entity_node->get_id())
	{
		selected_flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (childrens.size() == 0)
	{
		selected_flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (depth < 2)
	{
		selected_flags |= ImGuiTreeNodeFlags_DefaultOpen;
	}
	std::string label = "";

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	if (entity_node->get_component<anim::PoseComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0.8f, 0.2f, 0.8f, 1.0f});

		ImGui::Text(ICON_MD_SETTINGS_ACCESSIBILITY);
	}
	else if (entity_node->get_component<anim::ArmatureComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0.2f, 0.8f, 0.8f, 1.0f});
		ImGui::Text(ICON_MD_SHARE);
	}
	else if (entity_node->get_component<anim::MeshComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0.8f, 0.8f, 0.2f, 1.0f});
		ImGui::Text(ICON_MD_VIEW_IN_AR);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0.1f, 0.1f, 0.1f, 1.0f});
		ImGui::Text(ICON_MD_LIST);
	}
	ImGui::SameLine();
	ImGui::PopStyleColor();
	label += " " + entity_node->get_name() + "##" + std::to_string(entity_node->get_id());
	bool node_open = ImGui::TreeNodeEx(label.c_str(), selected_flags);
	ImGui::PopStyleVar();

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		selected_entity = entity_node;
	}

	if (node_open)
	{
		for (size_t i = 0; i < childrens.size(); i++)
		{
			anim::Entity* selected_child_node = draw_tree_node(childrens[i].get(), node_flags, depth + 1);
			if (selected_child_node != nullptr)
			{
				selected_entity = selected_child_node;
			}
		}
		ImGui::TreePop();
	}
	return selected_entity;
}
}	 // namespace ui