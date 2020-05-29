#pragma once

#pragma once
#include "TextureMap.h"


namespace CHR
{
	const glm::vec3 G[] = {
		{1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
		{1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
		{0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1},
		{1,1,0}, {-1,1,0}, {0,-1,1}, {0,-1,-1}
	};//Noise gradients

	class NoiseTextureMap : public TextureMap
	{
	public:
		NoiseTextureMap(DECAL_M d_mode, bool conv_lin = true);


		glm::vec3 SampleAt(glm::vec3 uv) const;
		glm::vec3 BumpAt(glm::vec3 p) const;

		inline bool IsConvertionLinear() { return m_lin_convertion; }
		inline unsigned int GetScale() { return m_scale; }

		inline void SetConvertion(bool l) { m_lin_convertion = l; }
		inline void SetScale(float n) { m_scale = n; }

		//std::shared_ptr<Texture> m_texture;
	private:
		const int N = 16;
		const float EPSILON = 0.001;

		bool m_lin_convertion = true;//else absval
		float m_scale = 1;
		unsigned int P[16];

		glm::vec3 GetGradient(glm::ivec3 ind) const;

		inline void DrawGUIHelper()
		{
			ImGui::Text("--Perlin Noise--");
			ImGui::Text("-Conversion-");
			int e = m_lin_convertion ? 1 : 0;
			ImGui::RadioButton("Abs. Value", &e, 0); ImGui::SameLine();
			ImGui::RadioButton("Linear", &e, 1);
			m_lin_convertion = e;
			ImGui::Separator();

			ImGui::DragFloat("Scale", &m_scale, 0.5, 0.0f);
		}
	};
}