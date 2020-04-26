#include "TextureMap.h"


namespace Chroma
{
	TextureMap::TextureMap(std::shared_ptr<Texture> tex, DECAL_M d_mode, bool bilinear_interp)
		: m_texture(tex), m_decal_mode(d_mode), m_interpolated(bilinear_interp)
	{}
}