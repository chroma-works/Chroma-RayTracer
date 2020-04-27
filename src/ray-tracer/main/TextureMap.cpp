#include "TextureMap.h"

namespace Chroma
{
	TextureMap::TextureMap(std::shared_ptr<Texture> tex, DECAL_M d_mode, bool bilinear_interp)
		: m_texture(tex), m_decal_mode(d_mode), m_interpolated(bilinear_interp)
	{}

	glm::vec3 TextureMap::ColorAt(glm::vec2 uv)
	{
		glm::vec2 s = { uv.x * m_texture->GetWidth(), uv.y * m_texture->GetHeigth() };
		if (m_interpolated)
		{
			unsigned int p = floor(s.x), q = floor(s.y);
			float dx = abs(s.x - p);
			float dy = abs(s.y - q);

			return (1 - dx) * (1 - dy) * m_texture->ColorAt({p, q}) + 
				(1 - dx) * (dy) * m_texture->ColorAt({p, q+1}) + 
				(dx) * (1 - dy) * m_texture->ColorAt({p+1, q}) + 
				(dx) * (dy) * m_texture->ColorAt({p+1, q+1});
		}
		else
			return m_texture->ColorAt({ round(s.x), round(s.y) });
	}
}