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
		inline unsigned int GetNormalizer() { return m_normalizer; }

		inline void SetInterpolation(bool interp) { m_interpolated = interp; }
		inline void SetNormalizer(unsigned int n) { m_normalizer = n; }

		std::shared_ptr<Texture> m_texture;
	private:
		bool m_interpolated;
		unsigned int m_normalizer = 255;
	};
}