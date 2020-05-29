#include "ProceduralTextureMap.h"

namespace CHR
{
	ProcedurelTextureMap::ProcedurelTextureMap(DECAL_M d_mode, glm::vec3 b, glm::vec3 w, float s, float o)
		: m_black_color(b), m_white_color(w), m_scale(s), m_offset(o)
	{
		m_type = SOURCE_T::procedural;
		m_decal_mode = d_mode;
	}

	glm::vec3 ProcedurelTextureMap::SampleAt(glm::vec3 uv) const
	{
		bool x = (int)((uv.x + m_offset) * m_scale) % 2;
		bool y = (int)((uv.y + m_offset) * m_scale) % 2;
		bool z = (int)((uv.z + m_offset) * m_scale) % 2;

		bool xorXY = x != y;

		if (xorXY != z)
		{
			return m_black_color; // black color
		}
		else
		{
			return m_white_color; // white color
		}
	}

	glm::vec3 ProcedurelTextureMap::BumpAt(glm::vec3 p) const
	{
		return glm::vec3();
	}
}

