#pragma once
#include "Texture.h"
#include <stdio.h>
#include <iostream>


namespace Chroma
{
	enum class DECAL_M { re_kd, bl_kd, bump, re_bg };
	class TextureMap
	{
	public:
		TextureMap(std::shared_ptr<Texture> tex, DECAL_M d_mode, bool bilinear_interp = true);
		//~TextureMap();

		inline bool IsInterpolated() { return m_interpolated; }
		inline DECAL_M GetDecalMode() { return m_decal_mode; }
		inline unsigned int GetNormalizer() { return m_normalizer; }

		inline void SetInterpolation(bool interp) { m_interpolated = interp; }
		inline void SetDecalMode(DECAL_M mode) { m_decal_mode = mode; }
		inline void SetNormalizer(unsigned int n) { m_normalizer = n; }

		std::shared_ptr<Texture> m_texture;
	private:
		DECAL_M m_decal_mode;
		bool m_interpolated;
		unsigned int m_normalizer = 255;

	};
}