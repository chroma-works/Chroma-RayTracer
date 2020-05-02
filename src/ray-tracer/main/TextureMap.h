#pragma once
#include <stdio.h>
#include <iostream>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	enum class DECAL_M { re_kd, bl_kd, re_all, re_no, bump, re_bg };
	enum class SOURCE_T { image, noise };
	class TextureMap
	{
	public:
		virtual glm::vec3 SampleAt(glm::vec3 p) const = 0;
		virtual glm::vec3 BumpAt(glm::vec3 p) const = 0;

		inline void SetBumpFactor( float bf) 
		{
			if (m_decal_mode == DECAL_M::bump)
				m_bump_factor = bf;
		}

		inline DECAL_M GetDecalMode() { return m_decal_mode; }
		inline SOURCE_T GetType() { return m_type; }
		inline float GetBumpFactor() 
		{ 
			if (m_decal_mode == DECAL_M::bump)
				return m_bump_factor;
			else
				return NAN;
		}
		//inline void SetDecalMode(DECAL_M mode) { m_decal_mode = mode; }
		
	protected:
		DECAL_M m_decal_mode;
		SOURCE_T m_type;

		float m_bump_factor = 1.0f; 
	};
}