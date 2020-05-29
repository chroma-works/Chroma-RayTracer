#include "Editor.h"
#include <thirdparty/glm/glm/glm.hpp>
#include <ray-tracer/accelerationStructures/BVH.h>

#define IMGUI_DEFINE_MATH_OPERATORS

static float wheel_y_offset = 0.8;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	wheel_y_offset += yoffset * 0.05;
}

namespace CHR
{
	CHR::Editor* s_instance = 0;
	Editor::Editor(Window* win, Scene* scene, Settings s)
		: m_settings(s)
	{
		if (!s_instance)
		{
			s_instance = this;

			m_window = win;
			m_scene = scene;
			m_render = false;

			m_settings.act_editor_cam_name = m_scene->m_cameras.begin()->first;
			m_settings.act_rt_cam_name = m_settings.act_editor_cam_name;

			ImGui::CreateContext();
			ImGui_ImplGlfw_InitForOpenGL(m_window->m_window_handle, true);
			ImGui_ImplOpenGL3_Init("#version 130");
			//set scroll back
			glfwSetScrollCallback(m_window->m_window_handle, scroll_callback);
			InitSkin();

			ray_tracer = new CHR::RayTracer();

			if (!ray_tracer)
				CH_ERROR("Failed to create Chrom Ray Tracer");

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
		//delete m_window;
	}

	void Editor::OnUpdate()
	{
		OnDraw();
		HandleKeyBoardNavigation();
		m_window->OnUpdate();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if(m_preview_render)
			m_scene->Render(m_scene->GetCamera(m_settings.act_editor_cam_name));
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
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowRounding = 4.0f;
		//style.FramePadding = ImVec2(4, style.FramePadding.y);
		style.WindowBorderSize = 0.2;
		style.FrameBorderSize = 0.02;


		style.Colors[ImGuiCol_Text] = CHR_COLOR::LIGHT_BLUE;
		style.Colors[ImGuiCol_TextDisabled] = CHR_COLOR::LIGHT_BLUE * 0.7f;
		style.Colors[ImGuiCol_WindowBg] = CHR_COLOR::DARK_CYAN;
		style.Colors[ImGuiCol_ChildWindowBg] = CHR_COLOR::DARK_CYAN * 1.2f;
		style.Colors[ImGuiCol_PopupBg] = CHR_COLOR::DARK_CYAN * 1.2f;
		style.Colors[ImGuiCol_Border] = CHR_COLOR::DARKER_PURPLE;
		style.Colors[ImGuiCol_BorderShadow] = CHR_COLOR::DARK_ORANGE;//DARK_PURPLE * 0.5;
		style.Colors[ImGuiCol_FrameBg] = CHR_COLOR::DARK_PURPLE * 0.35f;
		style.Colors[ImGuiCol_FrameBgHovered] = CHR_COLOR::DARK_PURPLE * 0.38f;
		style.Colors[ImGuiCol_FrameBgActive] = CHR_COLOR::DARK_PURPLE * 0.4f;
		style.Colors[ImGuiCol_TitleBg] = CHR_COLOR::DARK_PURPLE;
		style.Colors[ImGuiCol_TitleBgCollapsed] = CHR_COLOR::DARK_PURPLE * 0.6;
		style.Colors[ImGuiCol_TitleBgActive] = CHR_COLOR::DARK_PURPLE * 1.1;
		style.Colors[ImGuiCol_MenuBarBg] = CHR_COLOR::DARK_PURPLE;
		style.Colors[ImGuiCol_ScrollbarBg] = CHR_COLOR::DARK_PURPLE * 0.35f;
		style.Colors[ImGuiCol_ScrollbarGrab] = CHR_COLOR::DARK_ORANGE;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = CHR_COLOR::DARK_ORANGE * 1.1f;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = CHR_COLOR::DARK_ORANGE * 1.2f;
		//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3f, 0.3f, 0.3f, 0.99f);
		style.Colors[ImGuiCol_CheckMark] = CHR_COLOR::DARK_ORANGE;
		style.Colors[ImGuiCol_SliderGrab] = CHR_COLOR::DARK_ORANGE;
		style.Colors[ImGuiCol_SliderGrabActive] = CHR_COLOR::DARK_ORANGE * 1.1f;
		style.Colors[ImGuiCol_Button] = CHR_COLOR::DARK_ORANGE;
		style.Colors[ImGuiCol_ButtonHovered] = CHR_COLOR::DARK_ORANGE * 1.1;
		style.Colors[ImGuiCol_ButtonActive] = CHR_COLOR::DARK_ORANGE * 1.2;

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
		style.Colors[ImGuiCol_TextSelectedBg] = CHR_COLOR::DARK_ORANGE;
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void Editor::DrawInspector()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Inspector", 0, flags);

		ImGui::SetWindowSize(ImVec2(320, 480));//ImGui::SetWindowSize(ImVec2(240, (m_window->GetHeight() - 20) / 2));
		ImGui::SetWindowPos(ImVec2(m_window->GetWidth() - 320, 0));

		if (selected_item_type != SELECTION_TYPE::none)
		{
			ImGui::Text("%s", selected_name.c_str());
			ImGui::Separator();
		}
		if (selected_item_type == SELECTION_TYPE::obj)
		{
			m_scene->m_scene_objects[selected_name]->DrawGUI();
		}
		else if (selected_item_type == SELECTION_TYPE::cam)
		{
			m_scene->m_cameras[selected_name]->DrawGUI();
			ImGui::Separator();
			bool tmp = m_settings.act_editor_cam_name.compare(selected_name) == 0;
			ImGui::Checkbox("Editor Camera", &tmp);
			m_settings.act_editor_cam_name = tmp ? selected_name : m_settings.act_editor_cam_name;

		}
		else if (selected_item_type == SELECTION_TYPE::light)
		{
			m_scene->m_lights[selected_name]->DrawGUI();
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
		static bool flag = true;
		m_settings.resolution = m_scene->GetCamera(m_settings.act_rt_cam_name)->GetResolution();
		if (flag)
		{
			//m_settings.recur_depth = m_scene->m_recur_dept;
			ray_tracer->m_settings.resolution = { -1.0f, -1.0f };
			flag = false;
			ImGui::SetNextWindowSize(ImVec2(820, 480));
			glGenTextures(1, &rendered_frame_texture_id);
		}

		if (m_settings.resolution != ray_tracer->m_settings.resolution)
		{
			ray_tracer->SetResoultion(m_settings.resolution);
			glGenTextures(1, &rendered_frame_texture_id);
			glBindTexture(GL_TEXTURE_2D, rendered_frame_texture_id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_settings.resolution.x, m_settings.resolution.y, 0, 
				GL_BGR, GL_UNSIGNED_BYTE, ray_tracer->m_rendered_image->GetPixels());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		if (m_render)
		{
			//m_scene->m_cameras[m_settings.act_rt_cam_name]->SetFocalDistance(m_scene->m_cameras[m_settings.act_rt_cam_name]->GetFocalDistance() + 1.0f);
			ray_tracer->Render(m_scene->m_cameras[m_settings.act_rt_cam_name],  *m_scene, false);
			glBindTexture(GL_TEXTURE_2D, rendered_frame_texture_id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_settings.resolution.x, m_settings.resolution.y,
				GL_RGB, GL_UNSIGNED_BYTE, ray_tracer->m_rendered_image->GetPixels());
		}

		ImGui::Begin("Ray Tracer", 0, ImGuiWindowFlags_None);
		ImGui::Text("Ray Traced Frame");
		float cw = ImGui::GetContentRegionAvailWidth() * 0.65f;

		ImGui::Image((ImTextureID)(intptr_t)rendered_frame_texture_id, 
			ImVec2(cw, cw * m_settings.resolution.y / m_settings.resolution.x));
		ImGui::SameLine();

		ImGui::BeginChild("Settings", ImVec2(0, 0));

		static std::string rt_mode_names[] = { "Ray Casting", "Recursive RT" };
		static RT_MODE selected_rt_method = RT_MODE::recursive_trace;


		if (ImGui::BeginCombo("RT Mode", rt_mode_names[selected_rt_method].c_str(), ImGuiComboFlags_None))
		{
			for (int i = 0; i < RT_MODE::size; i++)
			{
				if (ImGui::Selectable(rt_mode_names[i].c_str(), static_cast<RT_MODE>(i) == selected_rt_method))
				{
					selected_rt_method = static_cast<RT_MODE>(i);
					ray_tracer->SetRenderMode(selected_rt_method);
				}
				if (static_cast<RT_MODE>(i) == selected_rt_method)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo("RT Camera", m_settings.act_rt_cam_name.c_str(), ImGuiComboFlags_None))
		{
			for (auto it = m_scene->m_cameras.begin(); it != m_scene->m_cameras.end(); it++)
			{
				bool is_selected = (m_settings.act_rt_cam_name.compare(it->first) == 0);
				if (ImGui::Selectable(m_settings.act_rt_cam_name.c_str(), is_selected))
					m_settings.act_rt_cam_name = it->first;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		if (ImGui::InputInt2("Resolution", (int*)&m_settings.resolution.x))
		{
			m_scene->GetCamera(m_settings.act_rt_cam_name)->SetResolution(m_settings.resolution);
			ray_tracer->SetResoultion(m_settings.resolution);
			glGenTextures(1, &rendered_frame_texture_id);
			glBindTexture(GL_TEXTURE_2D, rendered_frame_texture_id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_settings.resolution.x, m_settings.resolution.y,
				0, GL_BGR, GL_UNSIGNED_BYTE, ray_tracer->m_rendered_image->GetPixels());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		ImGui::PushItemWidth(100);
		ImGui::InputInt("Recursion Depth", &m_settings.recur_depth);
		int n_sample = m_scene->GetCamera(m_settings.act_rt_cam_name)->GetNumberOfSamples();
		ImGui::InputInt("# of Samples ", &n_sample, 1 ,100);
		m_scene->GetCamera(m_settings.act_rt_cam_name)->SetNumberOfSamples(n_sample);

		ImGui::InputInt("Thread Count", &m_settings.thread_count);
		ImGui::PopItemWidth();
		ImGui::Separator();

		bool chng_color = false;
		if (m_render)
		{
			chng_color = true;
			ImGui::PushStyleColor(ImGuiCol_Button, CHR_COLOR::DARK_PURPLE);
		}
		if (ImGui::Button("Toggle Render"))
		{
			if(m_scene->m_accel_structure)
				m_render = !m_render;
			else
				CH_FATAL("Acceleration structure is NOT initialized");
		}
		if (chng_color)
		{
			ImGui::PopStyleColor();
			chng_color = false;
		}
		ImGui::SameLine();

		if (ImGui::Button("Render once & Save"))
		{
			if (m_scene->m_accel_structure)
			{
				ray_tracer->Render(m_scene->m_cameras[m_settings.act_rt_cam_name], *m_scene);
				std::string file_name = "../../assets/screenshots/" + m_scene->GetCamera(m_settings.act_rt_cam_name)->GetImageName();
				ray_tracer->m_rendered_image->SaveToDisk(file_name.c_str());
				glBindTexture(GL_TEXTURE_2D, rendered_frame_texture_id);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ray_tracer->m_settings.resolution.x, ray_tracer->m_settings.resolution.y, GL_RGB,
					GL_UNSIGNED_BYTE, ray_tracer->m_rendered_image->GetPixels());
			}
			else
				CH_FATAL("Acceleration structure is NOT initialized");
		}
		if (ImGui::Button("Save Frame"))
		{
			std::string file_name = "../../assets/screenshots/" + m_scene->GetCamera(m_settings.act_rt_cam_name)->GetImageName();
			ray_tracer->m_rendered_image->SaveToDisk(file_name.c_str());
		}
		static bool save_exr = m_settings.ldr_post_process;
		static int e = m_settings.ldr_post_process;
		if (ImGui::Checkbox("Save HDR Image(.exr)", &save_exr))
		{
			//e = m_settings.ldr_post_process;
			//m_settings.ldr_post_process = save_exr;
			delete ray_tracer->m_rendered_image;
			ray_tracer->m_rendered_image = new Image(m_settings.resolution.x, m_settings.resolution.y, save_exr);
		}
		if (save_exr)
		{
			static bool flag = true;
			if (flag)
			{
				e = 2;
				flag = false;
			}
			ImGui::Text("-LDR Image Options-");
			ImGui::RadioButton("Clamp pixels", &e, 1); ImGui::SameLine();
			ImGui::RadioButton("Tone Map", &e, 2);
			m_settings.ldr_post_process = (IM_POST_PROC_T)e;
		}

		ImGui::Separator();
		static std::string split_names[] = { "SAH", "HLBVH", "Middle", "Eq. counts" };
		static int selected_split = 0;

		static int max_num_prim = 1;
		ImGui::PushItemWidth(120);
		if (ImGui::BeginCombo("Split type", split_names[selected_split].c_str(), ImGuiComboFlags_None))
		{
			for (int i = 0; i < static_cast<int>(SplitMethod::count); i++)
			{
				if (ImGui::Selectable(split_names[i].c_str(), i == selected_split))
				{
					selected_split = i;
				}
				if (i == selected_split)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::InputInt("Max. # of prims", &max_num_prim);
		ImGui::PopItemWidth();

		if (ImGui::Button("Init BVH"))
		{
			m_scene->InitBVH(max_num_prim, static_cast<SplitMethod>(selected_split));
			CH_INFO("BVH initialized");
		}


		ImGui::Separator();

		ImGui::PushItemWidth(120);
		ImGui::DragFloat("Pertub. Bias", &m_settings.shadow_eps, 0.00001f, 0.0f, 0.8, "%.6f");
		ImGui::PopItemWidth();

		ImGui::Checkbox("Shadows", &m_settings.calc_shadows);
		ImGui::SameLine();

		if (selected_rt_method != RT_MODE::ray_cast)
		{
			ImGui::Checkbox("Reflections", &m_settings.calc_reflections);
			ImGui::SameLine();
			ImGui::Checkbox("Refractions", &m_settings.calc_refractions);
		}

		ImGui::Separator();

		ImGui::EndChild();
		ImGui::End();

		ray_tracer->SetResoultion(m_settings.resolution);
		ray_tracer->m_settings = m_settings;
	}
	void Editor::DrawSceneInfo()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Scene", 0, flags);

		ImGui::SetWindowSize(ImVec2(320, m_window->GetHeight() - 480));
		ImGui::SetWindowPos(ImVec2(m_window->GetWidth() - 320, 480));


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
		if (ImGui::CollapsingHeader("Lights"))
		{
			i = 0;
			for (auto element : m_scene->m_lights)
			{
				std::string name = element.first;
				ImGui::PushID(i);

				if (ImGui::Selectable(name.c_str(), selected_name == name.c_str()))
				{
					selected_item_type = SELECTION_TYPE::light;
					selected_name = name;
				}
				i++;
				ImGui::PopID();
			}
		}
		/*if (flag)
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
		}*/
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
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		ImGui::Begin("Editor Info", 0, flags);
		ImGui::SetWindowPos(ImVec2(5, 5));
		ImGui::Text("Camera Movement Speed: %f", m_settings.camera_move_speed);
		ImGui::SetWindowPos(ImVec2(5, 10));
		ImGui::Text("Camera Rotation Speed: %f", m_settings.camera_rotate_speed);
		ImGui::Checkbox("Preview Render", &m_preview_render);
		ImGui::End();
	}
	float yaw = -90.0f;
	float pitch = 0.0f;
	void Editor::HandleKeyBoardNavigation()
	{
		auto cam = m_scene->m_cameras[m_settings.act_editor_cam_name];
		glm::vec3 forward = glm::normalize(cam->GetGaze());
		glm::vec3 right = glm::cross(forward, glm::normalize(cam->GetUp()));

		if (ImGui::GetIO().KeysDown[GLFW_KEY_W])
		{
			cam->SetPosition(cam->GetPosition() + forward * m_settings.camera_move_speed);
			cam->SetGaze( forward);
		}

		else if (ImGui::GetIO().KeysDown[GLFW_KEY_S])
		{
			cam->SetPosition(cam->GetPosition() - forward * m_settings.camera_move_speed);
			cam->SetGaze(forward);
		}

		if (ImGui::GetIO().KeysDown[GLFW_KEY_A])
		{
			cam->SetPosition(cam->GetPosition() - glm::normalize(right) * m_settings.camera_move_speed);
			cam->SetGaze( forward);
		}

		else if (ImGui::GetIO().KeysDown[GLFW_KEY_D])
		{
			cam->SetPosition(cam->GetPosition() + glm::normalize(right) * m_settings.camera_move_speed);
			cam->SetGaze(forward);
		}

		if (ImGui::GetIO().KeysDown[GLFW_KEY_SPACE] && ImGui::GetIO().KeyAlt)
		{
			cam->SetPosition(cam->GetPosition() - glm::normalize(cam->GetUp()) * m_settings.camera_move_speed);
			//cam->SetGaze(forward);
		}
		else if (ImGui::GetIO().KeysDown[GLFW_KEY_SPACE])
		{
			cam->SetPosition(cam->GetPosition() + glm::normalize(cam->GetUp()) * m_settings.camera_move_speed);
			//cam->SetGaze(forward);
		}

		if (ImGui::GetIO().KeyAlt && ImGui::IsMouseDragging(0) &&
			ImGui::GetMouseDragDelta().x != 0.0 && ImGui::GetMouseDragDelta().y != 0.0)
		{
			glm::vec4 m_d;
			m_d = glm::vec4(ImGui::GetMouseDragDelta().x, ImGui::GetMouseDragDelta().y, 0.1, 1.0);

			float xoffset = ImGui::GetMouseDragDelta(0, 1.0).x; 
			float yoffset = ImGui::GetMouseDragDelta(0, 1.0).y; 

			float sensitivity = m_settings.camera_rotate_speed*0.05;
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
			m_settings.camera_rotate_speed = glm::max(0.0f, m_settings.camera_rotate_speed + wheel_y_offset * 0.1f);
		else
			m_settings.camera_move_speed = glm::max(0.0f, m_settings.camera_move_speed + wheel_y_offset);
		wheel_y_offset = 0.0f;
	}
}