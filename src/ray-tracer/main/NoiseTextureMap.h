#pragma once

#pragma once
#include "TextureMap.h"


namespace Chroma
{
	const glm::vec3 G[] = {
		{1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
		{1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
		{0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1},
		{1,1,0}, {-1,1,0}, {0,-1,1}, {0,-1,-1}
	};//Noise gradients

	const int N = 16;

	class NoiseTextureMap : public TextureMap
	{
	public:
		NoiseTextureMap(DECAL_M d_mode, bool conv_lin = true);


		glm::vec3 SampleAt(glm::vec3 uv) const;

		inline bool IsConventionLinear() { return m_lin_convertion; }
		inline unsigned int GetScale() { return m_scale; }

		inline void SetConvertion(bool l) { m_lin_convertion = l; }
		inline void SetScale(float n) { m_scale = n; }

		//std::shared_ptr<Texture> m_texture;
	private:
		bool m_lin_convertion = true;//else absval
		float m_scale = 10;
		unsigned int P[16];

		glm::vec3 GetGradient(glm::ivec3 ind) const;
	};
}