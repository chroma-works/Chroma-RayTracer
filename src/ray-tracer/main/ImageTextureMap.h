#pragma once
#include "TextureMap.h"
#include "Texture.h"

namespace CHR
{
	class ImageTextureMap : public TextureMap
	{
	public:
		ImageTextureMap(std::shared_ptr<Texture> tex, DECAL_M d_mode, bool bilinear_interp = true);

		glm::vec3 SampleAt(glm::vec3 uv) const;
		glm::vec3 BumpAt(glm::vec3 p) const;

		inline bool IsInterpolated() { return m_interpolated; }
		inline bool IsDegammaOn() { return m_degamma; }
		inline unsigned int GetNormalizer() { return m_normalizer; }

		inline void SetInterpolation(bool interp) { m_interpolated = interp; }
		inline void SetDegamma(bool degamm) { m_degamma = degamm; }
		inline void SetNormalizer(unsigned int n) { m_normalizer = n; }


		std::shared_ptr<Texture> m_texture;
	private:
		bool m_degamma = false;
		bool m_interpolated;
		unsigned int m_normalizer = 255;

		inline void DrawGUIHelper()
		{
			ImGui::Text("--Image--");
			char* a = new char[m_texture->GetFilePath().size()];
			strcpy(a, m_texture->GetFilePath().c_str());
			ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
			if (ImGui::InputText("source", a, 128, flags))
			{
				if (std::string(a).compare(m_texture->GetFilePath()) != 0)
				{
					m_texture = std::make_shared<Texture>(std::string(a));
				}
			}
			delete[] a;
			int tmp = m_normalizer;
			ImGui::PushItemWidth(120);
			ImGui::InputInt("Normalizer", &tmp, 1, INT_MAX, ImGuiInputTextFlags_AutoSelectAll);
			m_normalizer = tmp;
			ImGui::PopItemWidth(); ImGui::SameLine();
			ImGui::Checkbox("Interpolate(bi-lin.)", &m_interpolated);

			ImGui::Separator();
		}
	};
}