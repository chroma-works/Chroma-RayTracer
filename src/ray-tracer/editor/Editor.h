#pragma once
#include <thirdparty/imgui/imgui.h>
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"
#include <ray-tracer/main/RayTracer.h>
#include <ray-tracer/main/Scene.h>
#include <ray-tracer/main/Window.h>
#include <string.h>



namespace Chroma
{
	struct Options
	{
		int thread_count = 1;
		std::string act_editor_cam_name = "";
		std::string act_rt_cam_name = "";
		glm::ivec2 editor_res;
		float m_camera_move_speed = 0.4;
		float m_camera_rotate_speed = 0.005;
	};
	class Editor
	{
	public:
		Editor(Window* window, Scene* scene);
		static Editor* getInstance();
		~Editor();

		Editor(Editor const&) = delete;
		void operator=(Editor const&) = delete;

		inline void SetScene(Scene* scene) { m_scene = scene; }
		inline void SetRayTracer(RayTracer* rt) { ray_tracer = rt; }

		void OnUpdate();
		void OnDraw();

	private:

		enum class SELECTION_TYPE{none, obj, cam, p_light, d_light, s_light};

		RayTracer* ray_tracer;

		Window* m_window;
		bool m_render;
		Scene* m_scene;

		Options m_options;

		unsigned int rendered_frame_texture_id;

		/*Camera* selected_cam;
		SceneObject* selected_obj;
		DirectionalLight* selected_dir_l;*/

		SELECTION_TYPE selected_item_type = SELECTION_TYPE::none;
		std::string selected_name;

		void InitSkin();
		void DrawInspector();
		void DrawRayTracedFrame();
		void DrawSceneInfo();
		void DrawEditorInfo();
		void HandleKeyBoardNavigation();
	};

}