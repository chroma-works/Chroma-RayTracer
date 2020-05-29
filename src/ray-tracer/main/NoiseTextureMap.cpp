#include "NoiseTextureMap.h"

#include <random>
#include <time.h>

namespace CHR
{

	float Weight(float x)
	{
		x = abs(x);
		return -6 * glm::pow(x, 5) + 15 * glm::pow(x, 4) - 10 * glm::pow(x, 3) + 1;
	}

	NoiseTextureMap::NoiseTextureMap(DECAL_M d_mode, bool conv_lin)
		:m_lin_convertion(conv_lin)
	{
		m_type = SOURCE_T::noise;
		m_decal_mode = d_mode;
		for (int i = 0; i < N; i++)
			P[i] = i;

		// Shuffling P array 
		std::shuffle(P, P + 16, std::default_random_engine(time(NULL)));
	}

	glm::vec3 NoiseTextureMap::GetGradient(glm::ivec3 ind) const
	{
		return G[ P[ (ind.x + P[(ind.y + P[ind.z % N]) % N]) % N]];
	}

	glm::vec3 NoiseTextureMap::SampleAt(glm::vec3 p) const
	{
		p = p * m_scale;
		float x_min = floor(p.x), x_max = ceil(p.x),
			y_min = floor(p.y), y_max = ceil(p.y),
			z_min = floor(p.z), z_max = ceil(p.z);

		glm::vec3 lattice[] = {
			{x_min, y_min, z_min}, {x_max, y_min, z_min},
			{x_min, y_max, z_min}, {x_min, y_min, z_max},
			{x_max, y_max, z_min}, {x_min, y_max, z_max},
			{x_max, y_min, z_max}, {x_max, y_max, z_max}
		};

		float value = 0;

		for (int i = 0; i < 8; i++)
		{
			lattice[i] = lattice[i];
			
			glm::vec3 v_i = (p - lattice[i]) ;
			value += glm::dot(GetGradient(glm::ivec3(lattice[i])), v_i) *
				(Weight(v_i.x) * Weight(v_i.y) * Weight(v_i.z));
		}

		value = m_lin_convertion ? (value + 1.0f) * 0.5f : abs(value);

		return { value,value,value };
	}

	glm::vec3 NoiseTextureMap::BumpAt(glm::vec3 p) const
	{
		glm::vec3 noise_o, noise_x, noise_y, noise_z;
		noise_o = SampleAt(p);
		noise_x = SampleAt(p + glm::vec3(EPSILON, 0, 0));
		noise_y = SampleAt(p + glm::vec3(0, EPSILON, 0));
		noise_z = SampleAt(p + glm::vec3(0, 0, EPSILON));

		return glm::vec3(
			(noise_x.x - noise_o.x) / EPSILON,
			(noise_y.x - noise_o.x) / EPSILON,
			(noise_z.x - noise_o.x) / EPSILON
		) * m_bump_factor;
	}

}