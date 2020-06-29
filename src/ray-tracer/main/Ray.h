#pragma once
#include <thirdparty/glm/glm/glm.hpp>
#include "Material.h"
#include "TextureMap.h"
#include "Light.h"

namespace CHR
{

	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;

		float intersect_eps = 0.0f;

		mutable float jitter_t = 0.0f;

		Ray(glm::vec3 orig = glm::vec3(0, 0, 0), glm::vec3 dir = glm::vec3(0, 0, 0))
			:origin(orig), direction(dir)
		{}

		Ray(const Ray& ray)
			:origin(ray.origin), direction(ray.direction), 
			intersect_eps(ray.intersect_eps), jitter_t(ray.jitter_t)
		{}


		glm::vec3 PointAt(float t) const { return origin + t * glm::normalize(direction); }
	};

	struct IntersectionData
	{
		float t = INFINITY;
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;

		Material* material;
		TextureMap* tex_map = nullptr;

		glm::vec3 radiance = { 0,0,0 };

		bool hit = false;

		glm::vec3 Shade(glm::vec3 radiance,
			 glm::vec3 e_vec, glm::vec3 l_vec)
		{
			glm::vec3 kd = material->m_diffuse;
			bool shade = true;

			if (tex_map)
			{
				glm::vec3 sample_point = tex_map->GetType() == SOURCE_T::image ?
					glm::vec3(uv, NAN) : position;

				switch (tex_map->GetDecalMode())
				{
				case DECAL_M::re_kd:
					kd = tex_map->SampleAt(sample_point);
					break;

				case DECAL_M::bl_kd:
					kd = kd * 0.5f + tex_map->SampleAt(sample_point) * 0.5f;
					break;

				case DECAL_M::re_all:
					shade = false;
					break;
				default:
					break;
				}
			}
			if (shade)
			{
				Material mat(*material);
				mat.m_diffuse = kd;

				return radiance * mat.Shade(l_vec, e_vec, normal) * glm::dot(normal, l_vec); // L * BRDF * cos_theta
			}
			else
				return tex_map->SampleAt(glm::vec3(uv, NAN));
		}
	};
}
