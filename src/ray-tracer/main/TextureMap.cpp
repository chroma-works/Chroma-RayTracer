#include "TextureMap.h"

namespace Chroma
{
	TextureMap::TextureMap(std::shared_ptr<Texture> tex, DECAL_M d_mode, bool bilinear_interp)
		: m_texture(tex), m_decal_mode(d_mode), m_interpolated(bilinear_interp)
	{}

	glm::vec3 TextureMap::SampleAt(glm::vec2 uv)
	{
		glm::vec2 s = { uv.x * m_texture->GetWidth(), uv.y * m_texture->GetHeigth() };
		glm::vec3 sample;
		if (m_interpolated)
		{
			unsigned int p = floor(s.x), q = floor(s.y);
			float dx = abs(s.x - p);
			float dy = abs(s.y - q);
			sample = glm::vec3((1 - dx) * (1 - dy) * m_texture->SampleAt({ p, q }) +
				(1 - dx) * (dy)*m_texture->SampleAt({ p, q + 1 }) +
				(dx) * (1 - dy) * m_texture->SampleAt({ p + 1, q }) +
				(dx) * (dy)*m_texture->SampleAt({ p + 1, q + 1 })) / ((float)m_normalizer);
		}
		else
		{
			sample = glm::vec3(m_texture->SampleAt({ round(s.x), round(s.y) })) / ((float)m_normalizer);
		}

		if (m_decal_mode == DECAL_M::re_no)
		{
			sample -= glm::vec3(0.5, 0.5, 0.5);
			sample = glm::normalize(sample);
		}
		return sample;
	}
}