#pragma once
#include <stdio.h>
#include <iostream>

#include <thirdparty/glm/glm/glm.hpp>
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"

namespace CHR
{
	enum class DECAL_M { re_kd, bl_kd, re_all, re_no, bump, re_bg };
	enum class SOURCE_T { image, noise, procedural};
	class TextureMap
	{
	public:
		virtual glm::vec3 SampleAt(glm::vec3 p) const = 0;
		virtual glm::vec3 BumpAt(glm::vec3 p) const = 0;

		inline void DrawGUI()
		{
			static std::string decal_names[] = { "Replace diffuse", "Blend wt. diffuse", 
				"Replace all", "Replace normals", "Bump map" };
			int selected_mode = static_cast<int>(m_decal_mode);

			ImGui::PushItemWidth(150);
			if (ImGui::BeginCombo("Decal Mode", decal_names[selected_mode].c_str(), ImGuiComboFlags_None))
			{
				for (int i = 0; i < 5; i++)
				{
					if (ImGui::Selectable(decal_names[i].c_str(), i == selected_mode))
					{
						selected_mode = i;
					}
					if (i == selected_mode)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			m_decal_mode = static_cast<DECAL_M>(selected_mode);

			ImGui::PopItemWidth();
			ImGui::Separator();
			DrawGUIHelper();
		}

		inline void SetBumpFactor( float bf) 
		{
			if (m_decal_mode == DECAL_M::bump)
				m_bump_factor = bf;
		}

		inline DECAL_M GetDecalMode() { return m_decal_mode; }
		inline SOURCE_T GetType() { return m_type; }
		inline float GetBumpFactor() 
		{ 
			if (m_decal_mode == DECAL_M::bump)
				return m_bump_factor;
			else
				return NAN;
		}
		//inline void SetDecalMode(DECAL_M mode) { m_decal_mode = mode; }
		
	protected:
		DECAL_M m_decal_mode;
		SOURCE_T m_type;

		float m_bump_factor = 1.0f; 

	private:
		virtual void DrawGUIHelper() = 0;
	};
}