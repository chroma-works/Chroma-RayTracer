#pragma once
#include <thirdparty/imgui/imgui.h>
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"
#include <ray-tracer/main/Window.h>


namespace CH_Editor
{
	class Editor
	{
	public:
		Editor(Window* window);
		static Editor* getInstance();
		~Editor();

		Editor(Editor const&) = delete;
		void operator=(Editor const&) = delete;


		void OnUpdate();
		void OnDraw();

	private:

		bool flag=true;

		Window* m_window;
		bool m_render;
		void InitSkin();
		void DrawInspector();
		void DrawRayTracedFrame();
	};

}