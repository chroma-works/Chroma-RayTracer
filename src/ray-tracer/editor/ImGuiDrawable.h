#pragma once
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"

namespace Chroma
{
	class ImGuiDrawable
	{
	public:
		virtual void DrawUI() = 0;
	};
}
