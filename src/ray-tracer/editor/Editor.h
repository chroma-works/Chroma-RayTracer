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
		~Editor();

		void OnUpdate();
		void OnDraw();
	private:
		Window* m_window;
		void InitSkin();
		void DrawInspector();
	};

}