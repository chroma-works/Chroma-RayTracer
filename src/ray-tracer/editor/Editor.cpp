#include "Editor.h"
#include <thirdparty/glm/glm/glm.hpp>
#include <string.h>


namespace CH_Editor
{
	CH_Editor::Editor* s_instance = 0;
	Editor::Editor(Window* win)
	{
		if (!s_instance)
		{
			s_instance = this;

			m_window = win;
			m_render = false;

			ImGui::CreateContext();
			ImGui_ImplGlfw_InitForOpenGL(m_window->m_window_handle, true);
			ImGui_ImplOpenGL3_Init("#version 130");
			InitSkin();

		}
		else
			CH_ERROR("Failed to create an instance of Editor");
	}

	Editor* Editor::getInstance()
	{
		if (s_instance != 0)
		{
			return s_instance;
		}
		else
			CH_ERROR("Editor is Null use constructor to create an instance");
		return s_instance;
	}

	Editor::~Editor()
	{
		delete m_window;
	}

	void Editor::OnUpdate()
	{
		OnDraw();
		m_window->OnUpdate();
	}

	void Editor::OnDraw()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		DrawInspector();
		DrawRayTracedFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	void Editor::InitSkin()
	{
		const ImVec4 DARK_ORANGE = ImVec4(0.65f, 0.33f, 0.13f, 1.0f);
		const ImVec4 LIGHT_BLUE = ImVec4(0.5f, 0.7f, 0.7f, 1.00f) * 1.2;
		const ImVec4 DARK_CYAN = ImVec4(0.1f, 0.2f, 0.2f, 1.00f);
		const ImVec4 DARKER_PURPLE = ImVec4(0.16f, 0.13f, 0.18f, 1.00f);
		const ImVec4 DARK_PURPLE = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowRounding = 4.0f;
		//style.FramePadding = ImVec2(4, style.FramePadding.y);
		style.WindowBorderSize = 0.2;
		style.FrameBorderSize = 0.02;


		style.Colors[ImGuiCol_Text] = LIGHT_BLUE;
		style.Colors[ImGuiCol_TextDisabled] = LIGHT_BLUE * 0.7f;
		style.Colors[ImGuiCol_WindowBg] = DARK_CYAN;
		style.Colors[ImGuiCol_ChildWindowBg] = DARK_CYAN * 1.2f;
		style.Colors[ImGuiCol_PopupBg] = DARK_CYAN * 1.2f;
		style.Colors[ImGuiCol_Border] = DARKER_PURPLE;
		style.Colors[ImGuiCol_BorderShadow] = DARK_ORANGE;//DARK_PURPLE * 0.5;
		style.Colors[ImGuiCol_FrameBg] = DARK_PURPLE * 0.35f;
		style.Colors[ImGuiCol_FrameBgHovered] = DARK_PURPLE * 0.38f;
		style.Colors[ImGuiCol_FrameBgActive] = DARK_PURPLE * 0.4f;
		style.Colors[ImGuiCol_TitleBg] = DARK_PURPLE;
		style.Colors[ImGuiCol_TitleBgCollapsed] = DARK_PURPLE * 0.6;
		style.Colors[ImGuiCol_TitleBgActive] = DARK_PURPLE * 1.1;
		style.Colors[ImGuiCol_MenuBarBg] = DARK_PURPLE;
		style.Colors[ImGuiCol_ScrollbarBg] = DARK_PURPLE * 0.35f;
		style.Colors[ImGuiCol_ScrollbarGrab] = DARK_ORANGE;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = DARK_ORANGE * 1.1f;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = DARK_ORANGE * 1.2f;
		//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3f, 0.3f, 0.3f, 0.99f);
		style.Colors[ImGuiCol_CheckMark] = DARK_ORANGE;
		style.Colors[ImGuiCol_SliderGrab] = DARK_ORANGE;
		style.Colors[ImGuiCol_SliderGrabActive] = DARK_ORANGE * 1.1f;
		style.Colors[ImGuiCol_Button] = DARK_ORANGE;
		style.Colors[ImGuiCol_ButtonHovered] = DARK_ORANGE * 1.1;
		style.Colors[ImGuiCol_ButtonActive] = DARK_ORANGE * 1.2;

		style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.35f, 0.35f, 1.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.4f, 0.4f, 1.0f);

		style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.8f, 0.4f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = DARK_ORANGE;
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void Editor::DrawInspector()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Inspector", 0, flags);

		ImGui::SetWindowSize(ImVec2(240, 240));//ImGui::SetWindowSize(ImVec2(240, (m_window->GetHeight() - 20) / 2));
		ImGui::SetWindowPos(ImVec2(m_window->GetWidth() - 240, 0));

		/*if (selected_scene_object)
		{
			ImGui::Checkbox("", &selected_scene_object->m_is_visible);

			ImGui::SameLine();

			ImGui::Text("%s", selected_scene_object->name.c_str());

			ImGui::Separator();

			ImGui::Text("Transform");

			if (ImGui::Button("P##1"))selected_scene_object->pos = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(selected_scene_object->pos.x), 0.05f, 0, 0, "%.3f");

			if (ImGui::Button("R##2"))selected_scene_object->rot = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##5", &(selected_scene_object->rot.x), 0.25f, 0, 0, "%.3f");

			if (ImGui::Button("S##3"))selected_scene_object->scale = glm::vec3(1, 1, 1);
			ImGui::SameLine();
			ImGui::DragFloat3("##6", &(selected_scene_object->scale.x), 0.05f, 0, 0, "%.3f");

			ImGui::Separator();

			if (selected_scene_object->light)
			{
				ImGui::Text("Light");
				ImGui::ColorEdit3("Color", &selected_scene_object->light->color.x);
				ImGui::DragFloat("Intensity", &selected_scene_object->light->intensity);
				ImGui::Separator();
			}

			if (selected_scene_object->mesh)
			{
				ImGui::Text("Mesh");

				ImGui::Text("Vertex count: %d", selected_scene_object->mesh->m_vertex_count);

				//if (ImGui::Button("Center Pivot Point"))
				//{
				//	selected_scene_object->mesh->CenterPivot();
				//	selected_scene_object->mesh->isDirty = true;
				//}

				ImGui::Checkbox("Ignore Tetrahedralization", &selected_scene_object->mesh->m_ignore_tetrahedralization);
				ImGui::Checkbox("Structure Mesh", &selected_scene_object->mesh->m_structure_mesh);

				ImGui::Separator();
			}

			if (selected_scene_object->material)
			{
				ImGui::Text("Material");
				ImGui::ColorEdit3("Diffuse", &selected_scene_object->material->diffuse.r);

				ImGui::Separator();

				Material* material = selected_scene_object->material;

				if (material->texture)
				{
					std::unordered_map<Texture*, GLuint>::const_iterator result = graphics->texture_handles.find(material->texture);

					if (result != graphics->texture_handles.end())
					{
						float aspect = (float)material->texture->w / material->texture->h;

						ImVec2 size(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvailWidth() / aspect);

						ImGui::Image((ImTextureID)(intptr_t)result->second, size);
					}
				}
			}

			//ImGui::ShowStyleEditor();

		}*/

		ImGui::Button("Remove Component");
		ImGui::End();
	}
	void Editor::DrawRayTracedFrame()
	{
		float f = 5.0f;
		glm::ivec2 DUMMY_RES = glm::ivec2(1080, 720);
		if (flag)
		{
			flag = false;
			ImGui::SetNextWindowSize(ImVec2(820, 480));
		}
		ImGui::Begin("Ray Tracer", 0, ImGuiWindowFlags_None);
		ImGui::Text( "Ray Traced Frame");
		float cw = ImGui::GetContentRegionAvailWidth() * 0.7f;


		ImGui::Image((ImTextureID)(intptr_t)200, ImVec2(cw, cw * DUMMY_RES.y / DUMMY_RES.x));
		ImGui::SameLine();

		ImGui::BeginChild("Settings", ImVec2(0, 0));
			ImGui::InputInt2("Resolution", (int*)&DUMMY_RES.x);
			if (ImGui::Button("Toggle Render"))
				m_render = !m_render;
		ImGui::EndChild();
		ImGui::End();
	}
}