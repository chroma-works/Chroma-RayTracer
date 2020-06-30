#include "ImageTextureMap.h"

#include <thirdparty/glm/glm/gtx/component_wise.hpp>


namespace CHR
{
	ImageTextureMap::ImageTextureMap(std::shared_ptr<Texture> tex, DECAL_M d_mode, bool bilinear_interp)
		: m_texture(tex), m_interpolated(bilinear_interp)                                    
	{
		m_type = SOURCE_T::image;
		m_decal_mode = d_mode;
	}

	glm::vec3 ImageTextureMap::SampleAt(glm::vec3 uv) const
	{
		glm::vec2 uv_r = { uv.x- floor(uv.x),  uv.y - floor(uv.y) };

		glm::vec2 s = { uv_r.x * m_texture->GetWidth(), uv_r.y * m_texture->GetHeigth() };
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
		if (m_degamma)
		{
			glm::vec3 g = { 2.2, 2.2, 2.2 };
			sample = glm::pow(sample, g);
		}
		return sample;
	}

	glm::vec3 ImageTextureMap::BumpAt(glm::vec3 uv) const
	{
		glm::vec2 uv_r = { uv.x - floor(uv.x),  uv.y - floor(uv.y) };

		glm::vec2 s = { uv_r.x * m_texture->GetWidth(), uv_r.y * m_texture->GetHeigth() };
		glm::vec3 sample = glm::vec3(m_texture->SampleAt({ floor(s.x), floor(s.y) }));
		glm::vec3 right_sample = glm::vec3(m_texture->SampleAt({ floor(s.x) + 1, floor(s.y) }));
		glm::vec3 upper_sample = glm::vec3(m_texture->SampleAt({ floor(s.x), floor(s.y) + 1 }));
		float du = glm::compAdd(right_sample - sample)/3.0f;
		float dv = glm::compAdd(upper_sample - sample)/3.0f;

		return glm::vec3(du/m_normalizer, dv/m_normalizer, NAN) * m_bump_factor;
	}
}