#pragma once

#include "TextureMap.h"
#include "Texture.h"


namespace CHR
{
	class ProcedurelTextureMap : public TextureMap
	{
	public:
		ProcedurelTextureMap(DECAL_M d_mode, 
			glm::vec3 b = glm::vec3( 0,0,0 ), glm::vec3 w = { 255,255,255 },
			float s = 5, float o = 0);

		glm::vec3 SampleAt(glm::vec3 uv) const;
		glm::vec3 BumpAt(glm::vec3 p) const;


		inline glm::vec3 GetBlack() { return m_black_color; }
		inline glm::vec3 GetWhite() { return m_white_color; }
		inline float GetScale() { return m_scale; }
		inline float GetOffset() { return m_offset; }

		inline void SetBlack(glm::vec3 c) { m_black_color = c; }
		inline void SetWhite(glm::vec3 c) { m_white_color = c; }
		inline void SetScale(float s) { m_scale = s; }
		inline void SetOffset(float o) { m_offset = o; }

	private:
		glm::vec3 m_black_color = {0,0,0};
		glm::vec3 m_white_color = { 255,255,255 };
		float m_scale = 5;
		float m_offset = 0;

		inline void DrawGUIHelper()
		{
			ImGui::Text("--Procedural--");

			ImGui::ColorEdit3("Dark color", &m_black_color.x, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit3("Light color", &m_white_color.x, ImGuiColorEditFlags_Float);
			ImGui::Separator();

			ImGui::PushItemWidth(90);
			ImGui::InputFloat("Scale", &m_scale, 0.05f, 0.0f); ImGui::SameLine();
			ImGui::InputFloat("Offset", &m_offset, 0.05f, 0.0f);
			ImGui::PopItemWidth();

		}
	};
}
