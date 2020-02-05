#include "Editor.h"



namespace CH_Editor
{
	Editor::Editor(Window* win)
	{
		m_window = win;

		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(m_window->m_window_handle, true);
		ImGui_ImplOpenGL3_Init("#version 130");

		InitSkin();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
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

		static float f = 0.0f;
		static int counter = 0;

		ImGui::NewFrame();
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	void Editor::InitSkin()
	{
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowRounding = 0.0f;
		//style.FramePadding = ImVec2(4, style.FramePadding.y);
	 //   style.WindowBorderSize = 1;
	 //   style.FrameBorderSize = 1;

		return;

		style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
		//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3f, 0.3f, 0.3f, 0.99f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.6f, 0.6f, 0.6f, 0.99f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6f, 0.6f, 0.6f, 0.99f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7f, 0.7f, 0.7f, 0.99f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);

		style.Colors[ImGuiCol_Header] = ImVec4(0.9f, 0.5f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.9f, 0.5f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.9f, 0.5f, 0.0f, 1.0f);

		style.Colors[ImGuiCol_Header] = ImVec4(0.17f, 0.57f, 0.69f, 1.0f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.57f, 0.69f, 1.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.17f, 0.57f, 0.69f, 1.0f);

		style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.8f, 0.4f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		/*style.Colors[ImGuiCol_Button] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);*/
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.8f, 0.4f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void Editor::DrawInspector()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

		//ImGui::GetStyle().WindowTitleAlign = ImVec2(0.025f, 0.5f);
		ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiCond_FirstUseEver);
		ImGui::Begin("Inspector", 0, flags);

		/*ImGui::SetWindowSize(ImVec2(240, 240));//ImGui::SetWindowSize(ImVec2(240, (m_window->GetHeight() - 20) / 2));
		ImGui::SetWindowPos(ImVec2(m_window->GetWidth() - 240, 20));

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

		//ImGui::Button("Add Component");
		ImGui::End();
	}
}