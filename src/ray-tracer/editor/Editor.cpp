#include "Editor.h"
#include <thirdparty/glm/glm/glm.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS

static float wheel_y_offset = 0.8;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	wheel_y_offset += yoffset * 0.05;
}

namespace Chroma
{
	Chroma::Editor* s_instance = 0;
	Editor::Editor(Window* win, Scene* scene)
	{
		if (!s_instance)
		{
			s_instance = this;

			m_window = win;
			m_scene = scene;
			m_render = false;

			act_editor_cam_name = m_scene->m_cameras.begin()->first;
			act_rt_cam_name = act_editor_cam_name;

			ImGui::CreateContext();
			ImGui_ImplGlfw_InitForOpenGL(m_window->m_window_handle, true);
			ImGui_ImplOpenGL3_Init("#version 130");
			//set scroll back
			glfwSetScrollCallback(m_window->m_window_handle, scroll_callback);
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
		HandleKeyBoardNavigation();
		m_window->OnUpdate();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_scene->Render(m_scene->GetCamera(act_editor_cam_name));
	}

	void Editor::OnDraw()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		DrawInspector();
		DrawRayTracedFrame();
		DrawSceneInfo();
		DrawEditorInfo();

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

		ImGui::SetWindowSize(ImVec2(240, 480));//ImGui::SetWindowSize(ImVec2(240, (m_window->GetHeight() - 20) / 2));
		ImGui::SetWindowPos(ImVec2(m_window->GetWidth() - 240, 0));

		if (selected_item_type != SELECTION_TYPE::none)
		{
			ImGui::Text("%s", selected_name.c_str());
			ImGui::Separator();
		}
		if (selected_item_type == SELECTION_TYPE::obj)
		{
			bool tmp = m_scene->m_scene_objects[selected_name]->IsVisibleInEditor();
			ImGui::Checkbox("Editor visibility", &tmp);
			m_scene->m_scene_objects[selected_name]->HideInEditor(tmp);

			bool tmp2 = m_scene->m_scene_objects[selected_name]->IsVisible();
			ImGui::Checkbox("RT visibility", &tmp2);
			m_scene->m_scene_objects[selected_name]->SetVisible(tmp2);
			ImGui::Separator();
			ImGui::Text("Transform");

			if (ImGui::Button("P##1"))m_scene->m_scene_objects[selected_name]->SetPosition(glm::vec3());
			ImGui::SameLine();
			glm::vec3 tmp_pos = m_scene->m_scene_objects[selected_name]->GetPosition();
			ImGui::DragFloat3("##4", &(tmp_pos.x), 0.05f, 0, 0, "%.3f");
			m_scene->m_scene_objects[selected_name]->SetPosition(tmp_pos);

			if (ImGui::Button("R##2"))m_scene->m_scene_objects[selected_name]->SetRotation(glm::vec3());
			ImGui::SameLine();
			glm::vec3 tmp_rot = glm::eulerAngles(m_scene->m_scene_objects[selected_name]->GetRotation());
			ImGui::DragFloat3("##5", &(tmp_rot.x), 0.25f, 0, 0, "%.3f");
			m_scene->m_scene_objects[selected_name]->SetRotation(glm::quat(tmp_rot));

			if (ImGui::Button("S##3")) m_scene->m_scene_objects[selected_name]->SetScale(glm::vec3(1, 1, 1));
			ImGui::SameLine();
			glm::vec3 tmp_sca = m_scene->m_scene_objects[selected_name]->GetScale();
			ImGui::DragFloat3("##6", &(tmp_sca.x), 0.05f, 0, 0, "%.3f");
			m_scene->m_scene_objects[selected_name]->SetScale(tmp_sca);
			ImGui::Separator();
		}
		else if (selected_item_type == SELECTION_TYPE::cam)
		{
			bool tmp = act_editor_cam_name.compare(selected_name) == 0;
			ImGui::Checkbox("Editor Camera", &tmp);
			act_editor_cam_name = tmp ? selected_name : act_editor_cam_name;

			ImGui::Separator();

			ImGui::Text("Transform");

			if (ImGui::Button("P##1"))m_scene->m_cameras[selected_name]->SetPosition(glm::vec3());
			ImGui::SameLine();
			glm::vec3 tmp_pos = m_scene->m_cameras[selected_name]->GetPosition();
			ImGui::DragFloat3("##4", &(tmp_pos.x), 0.05f, 0, 0, "%.3f");
			m_scene->m_cameras[selected_name]->SetPosition(tmp_pos);

			if (ImGui::Button("G##2"))m_scene->m_cameras[selected_name]->SetGaze(glm::vec3());
			ImGui::SameLine();
			glm::vec3 tmp_gaze = m_scene->m_cameras[selected_name]->GetGaze();
			ImGui::DragFloat3("##5", &(tmp_gaze.x), 0.05f, 0, 0, "%.3f");
			m_scene->m_cameras[selected_name]->SetGaze(glm::normalize(tmp_gaze));

			if (ImGui::Button("U##3"))m_scene->m_cameras[selected_name]->SetUp(glm::vec3(0.0, 1.0, 0.0));
			ImGui::SameLine();
			glm::vec3 tmp_up = m_scene->m_cameras[selected_name]->GetUp();
			ImGui::DragFloat3("##6", &(tmp_up.x), 0.05f, 0, 0, "%.3f");
			m_scene->m_cameras[selected_name]->SetUp(glm::normalize(tmp_up));

			ImGui::Separator();
			char* tmp_buf = strdup(m_scene->m_cameras[selected_name]->GetImageName().c_str());
			ImGui::InputText("Image Name", tmp_buf, 20, 0);
			m_scene->m_cameras[selected_name]->SetImageName(tmp_buf);
		}
		else if (selected_item_type == SELECTION_TYPE::p_light)
		{
			ImGui::Text("Transform");
			if (ImGui::Button("P##1"))m_scene->m_point_lights[selected_name]->position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(m_scene->m_point_lights[selected_name]->position.x), 0.05f, 0, 0, "%.3f");

			ImGui::Separator();

			ImGui::Text("Light");
			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::ColorEdit3("Ambient Color", &m_scene->m_point_lights[selected_name]->ambient.x, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit3("Diffuse Color", &m_scene->m_point_lights[selected_name]->diffuse.x, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit3("Specular Color", &m_scene->m_point_lights[selected_name]->specular.x, ImGuiColorEditFlags_Float);
			

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = m_scene->m_point_lights[selected_name]->intensity;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				m_scene->m_point_lights[selected_name]->SetIntensity(tmp);
				/*glm::vec3 tmp = m_scene->m_point_lights[selected_name]->intensity / 1000.0f;
				m_scene->m_point_lights[selected_name]->ambient =
					m_scene->m_point_lights[selected_name]->diffuse =
					m_scene->m_point_lights[selected_name]->specular = tmp;*/
			}
		}
		else if (selected_item_type == SELECTION_TYPE::d_light)
		{
			ImGui::Text("Transform");
			if (ImGui::Button("D##1"))m_scene->m_dir_lights[selected_name]->direction = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(m_scene->m_dir_lights[selected_name]->direction.x), 0.05f, 0, 0, "%.3f");

			ImGui::Separator();

			ImGui::Text("Light");
			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);
			ImGui::ColorEdit3("Ambient Color", &m_scene->m_dir_lights[selected_name]->ambient.x);
			ImGui::ColorEdit3("Diffuse Color", &m_scene->m_dir_lights[selected_name]->diffuse.x);
			ImGui::ColorEdit3("Specular Color", &m_scene->m_dir_lights[selected_name]->specular.x);

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = m_scene->m_dir_lights[selected_name]->intensity;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				m_scene->m_dir_lights[selected_name]->SetIntensity(tmp);
				/*glm::vec3 tmp = m_scene->m_dir_lights[selected_name]->intensity / 1000.0f;
				m_scene->m_dir_lights[selected_name]->ambient =
					m_scene->m_dir_lights[selected_name]->diffuse =
					m_scene->m_dir_lights[selected_name]->specular = tmp;*/
			}


		}
		else if (selected_item_type == SELECTION_TYPE::s_light)
		{
			ImGui::Text("Transform");
			if (ImGui::Button("P##1"))m_scene->m_spot_lights[selected_name]->position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##5", &(m_scene->m_spot_lights[selected_name]->position.x), 0.05f, 0, 0, "%.3f");
			if (ImGui::Button("D##2"))m_scene->m_spot_lights[selected_name]->direction = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##6", &(m_scene->m_spot_lights[selected_name]->direction.x), 0.05f, 0, 0, "%.3f");

			ImGui::Separator();

			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::Text("Light");
			ImGui::ColorEdit3("Ambient Color", &m_scene->m_spot_lights[selected_name]->ambient.x);
			ImGui::ColorEdit3("Diffuse Color", &m_scene->m_spot_lights[selected_name]->diffuse.x);
			ImGui::ColorEdit3("Specular Color", &m_scene->m_spot_lights[selected_name]->specular.x);

			ImGui::Separator();

			if (ImGui::Button("Cut-off##3"))m_scene->m_spot_lights[selected_name]->cutOff = 0.1;
			ImGui::SameLine();
			ImGui::DragFloat("##7", &(m_scene->m_spot_lights[selected_name]->cutOff), 0.05f, 0, 0, "%.3f");
			if (ImGui::Button("O.Cut-off##4"))m_scene->m_spot_lights[selected_name]->outerCutOff = 0.5;
			ImGui::SameLine();
			ImGui::DragFloat("##8", &(m_scene->m_spot_lights[selected_name]->outerCutOff), 0.05f, 0, 0, "%.3f");

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = m_scene->m_spot_lights[selected_name]->intensity;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				m_scene->m_spot_lights[selected_name]->SetIntensity(tmp);
				/*m_scene->m_spot_lights[selected_name]->ambient =
					m_scene->m_spot_lights[selected_name]->diffuse =
					m_scene->m_spot_lights[selected_name]->specular = tmp;*/
			}
		}


		if (ImGui::Button("Remove Component") && selected_item_type!=SELECTION_TYPE::cam) 
		{ 
			m_scene->m_scene_objects.erase(selected_name);
			/*m_scene->m_dir_lights.erase(selected_name);
			m_scene->m_point_lights.erase(selected_name);
			m_scene->m_spot_lights.erase(selected_name);*/
			selected_item_type = SELECTION_TYPE::none;
		};
		ImGui::End();
	}
	void Editor::DrawRayTracedFrame()
	{
		static glm::ivec2 res = ray_tracer->m_resolution;
		static bool flag = true;
		if (flag)
		{
			flag = false;
			ImGui::SetNextWindowSize(ImVec2(820, 480));
			glGenTextures(1, &rendered_frame_texture_id);
		}
		ImGui::Begin("Ray Tracer", 0, ImGuiWindowFlags_None);
		ImGui::Text( "Ray Traced Frame");
		float cw = ImGui::GetContentRegionAvailWidth() * 0.65f;

		ImGui::Image((ImTextureID)(intptr_t)rendered_frame_texture_id, ImVec2(cw, cw * ray_tracer->m_resolution.y / ray_tracer->m_resolution.x));
		ImGui::SameLine();

		ImGui::BeginChild("Settings", ImVec2(0, 0));
		ImGui::InputInt2("Resolution", (int*)&res.x);
		if (ImGui::Button("Toggle Render"))
			m_render = !m_render;
		if (ImGui::Button("Save Frame"))
		{
			std::string file_name = "../../assets/screenshots/" + m_scene->GetCamera(act_rt_cam_name)->GetImageName() + ".png";
			ray_tracer->m_rendered_image->SaveToDisk(file_name.c_str());
		}

		ImGui::EndChild();

		if (res != ray_tracer->m_resolution)
		{
			ray_tracer->SetResoultion(res);
			glBindTexture(GL_TEXTURE_2D, rendered_frame_texture_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ray_tracer->m_resolution.x, ray_tracer->m_resolution.y, 0, GL_BGR, GL_UNSIGNED_BYTE, ray_tracer->m_rendered_image->GetPixels());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		if (m_render)
		{
			ray_tracer->Render(m_scene->m_cameras[act_rt_cam_name],  *m_scene);
			glBindTexture(GL_TEXTURE_2D, rendered_frame_texture_id);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ray_tracer->m_resolution.x, ray_tracer->m_resolution.y, GL_BGR, GL_UNSIGNED_BYTE, ray_tracer->m_rendered_image->GetPixels());
		}
		ImGui::End();
	}
	void Editor::DrawSceneInfo()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Scene", 0, flags);

		ImGui::SetWindowSize(ImVec2(240, m_window->GetHeight() - 480));//ImGui::SetWindowSize(ImVec2(240, (m_window->GetHeight() - 20) / 2));
		ImGui::SetWindowPos(ImVec2(m_window->GetWidth() - 240, 480));


		ImGui::Text("RT Ambient");
		ImGui::SameLine();
		ImGui::DragFloat3("", &m_scene->m_ambient_l.x, 1.0f, 0.0f, 1000.0f);

		int i = 0;
		static bool flag = true;
		if(flag)
			ImGui::SetNextTreeNodeOpen(true);
		if (ImGui::CollapsingHeader("Objects"))
		{
			for (auto element : m_scene->m_scene_objects)
			{
				std::string name = element.first;
				ImGui::PushID(i);

				if (ImGui::Selectable(name.c_str(), selected_name == name.c_str()))
				{
					selected_item_type = SELECTION_TYPE::obj;
					selected_name = name;
				}
				i++;
				ImGui::PopID();
			}
		}
		if (flag)
			ImGui::SetNextTreeNodeOpen(true);
		if (ImGui::CollapsingHeader("Point Lights"))
		{
			i = 0;
			for (auto element : m_scene->m_point_lights)
			{
				std::string name = element.first;
				ImGui::PushID(i);

				if (ImGui::Selectable(name.c_str(), selected_name == name.c_str()))
				{
					selected_item_type = SELECTION_TYPE::p_light;
					selected_name = name;
				}
				i++;
				ImGui::PopID();
			}
		}
		if (flag)
			ImGui::SetNextTreeNodeOpen(true);
		if (ImGui::CollapsingHeader("Directional Lights"))
		{
			i = 0;
			for (auto element : m_scene->m_dir_lights)
			{
				std::string name = element.first;
				ImGui::PushID(i);

				if (ImGui::Selectable(name.c_str(), selected_name == name.c_str()))
				{
					selected_item_type = SELECTION_TYPE::d_light;
					selected_name = name;
				}
				i++;
				ImGui::PopID();
			}
		}
		if (flag)
			ImGui::SetNextTreeNodeOpen(true);
		if (ImGui::CollapsingHeader("Spot Lights"))
		{
			i = 0;
			for (auto element : m_scene->m_spot_lights)
			{
				std::string name = element.first;
				ImGui::PushID(i);

				if (ImGui::Selectable(name.c_str(), selected_name == name.c_str()))
				{
					selected_item_type = SELECTION_TYPE::s_light;
					selected_name = name;
				}
				i++;
				ImGui::PopID();
			}
		}
		if (flag)
		{
			ImGui::SetNextTreeNodeOpen(true);
			flag = false;
		}
		if (ImGui::CollapsingHeader("Cameras"))
		{
			i = 0;
			for (auto element : m_scene->m_cameras)
			{
				std::string name = element.first;
				ImGui::PushID(i);

				if (ImGui::Selectable(name.c_str(), selected_name == name.c_str()))
				{
					selected_item_type = SELECTION_TYPE::cam;
					selected_name = name;
				}
				i++;
				ImGui::PopID();
			}
		}

		ImGui::End();
	}
	void Editor::DrawEditorInfo()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;
		ImGui::Begin("Editor Info", 0, flags);
		ImGui::SetWindowPos(ImVec2(5, 5));
		ImGui::Text("Camera Movement Speed: %f", m_camera_move_speed);
		ImGui::SetWindowPos(ImVec2(5, 10));
		ImGui::Text("Camera Rotation Speed: %f", m_camera_rotate_speed);
		ImGui::End();
	}
	float yaw = -90.0f;
	float pitch = 180.0f;
	void Editor::HandleKeyBoardNavigation()
	{
		auto cam = m_scene->m_cameras[act_editor_cam_name];
		glm::vec3 forward = glm::normalize(cam->GetGaze());
		glm::vec3 right = glm::cross(forward, glm::normalize(cam->GetUp()));

		if (ImGui::GetIO().KeysDown[GLFW_KEY_W])
		{
			cam->SetPosition(cam->GetPosition() + forward * m_camera_move_speed);
			cam->SetGaze( forward);
		}

		else if (ImGui::GetIO().KeysDown[GLFW_KEY_S])
		{
			cam->SetPosition(cam->GetPosition() - forward * m_camera_move_speed);
			cam->SetGaze(forward);
		}

		if (ImGui::GetIO().KeysDown[GLFW_KEY_A])
		{
			cam->SetPosition(cam->GetPosition() - glm::normalize(right) * m_camera_move_speed);
			cam->SetGaze( forward);
		}

		else if (ImGui::GetIO().KeysDown[GLFW_KEY_D])
		{
			cam->SetPosition(cam->GetPosition() + glm::normalize(right) * m_camera_move_speed);
			cam->SetGaze(forward);
		}

		if (ImGui::GetIO().KeyAlt && ImGui::IsMouseDragging(0) &&
			ImGui::GetMouseDragDelta().x != 0.0 && ImGui::GetMouseDragDelta().y != 0.0)
		{
			glm::vec4 m_d;
			m_d = glm::vec4(ImGui::GetMouseDragDelta().x, ImGui::GetMouseDragDelta().y, 0.1, 1.0);

			float xoffset = ImGui::GetMouseDragDelta(0, 1.0).x; 
			float yoffset = ImGui::GetMouseDragDelta(0, 1.0).y; 

			float sensitivity = m_camera_rotate_speed*0.05;
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			yaw += xoffset;
			pitch += yoffset;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			glm::vec3 gaze =  glm::vec3(cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw));
			cam->SetGaze(gaze);
		}

		if(ImGui::GetIO().KeyAlt)
			m_camera_rotate_speed = glm::max(0.0f, m_camera_rotate_speed + wheel_y_offset * 0.1f);
		else
			m_camera_move_speed = glm::max(0.0f, m_camera_move_speed + wheel_y_offset);
		wheel_y_offset = 0.0f;
	}
}