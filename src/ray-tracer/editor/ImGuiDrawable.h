#pragma once
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"

namespace CHR
{
	class ImGuiDrawable
	{
	public:
		virtual void DrawGUI() = 0;

		inline std::string GetName() const { return m_name; }

		inline void SetName(const std::string n ) { m_name = n; }
	protected:
		std::string m_name;
	};
}

namespace CHR_COLOR
{
	const ImVec4 DARK_ORANGE = ImVec4(0.65f, 0.33f, 0.13f, 1.0f);
	const ImVec4 LIGHT_BLUE = ImVec4(0.5f, 0.7f, 0.7f, 1.00f) * 1.2;
	const ImVec4 DARK_CYAN = ImVec4(0.1f, 0.2f, 0.2f, 1.00f);
	const ImVec4 DARKER_PURPLE = ImVec4(0.16f, 0.13f, 0.18f, 1.00f);
	const ImVec4 DARK_PURPLE = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);

	const ImVec4 L_FRAME = ImVec4(0.54, 0.38, 0.0, 1.0);
	const ImVec4 O_FRAME = ImVec4(0.09, 0.39, 0.43, 1.0f);
	const ImVec4 C_FRAME = ImVec4(0.68, 0.25, 0.35, 1.0f);
}
