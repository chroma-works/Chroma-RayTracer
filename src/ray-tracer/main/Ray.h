#pragma once
#include <thirdparty/glm/glm/glm.hpp>
#include "Material.h"
#include "TextureMap.h"
#include "Light.h"

namespace Chroma
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

		bool hit = false;

		glm::vec3 Shade(std::shared_ptr<PointLight> pl,
			glm::vec3 l_vec, glm::vec3 e_vec)
		{
			glm::vec3 kd = material->m_diffuse;

			if (tex_map)
			{
				switch (tex_map->GetDecalMode())
				{
				case DECAL_M::re_kd:
					kd = tex_map->SampleAt(uv);
					break;

				case DECAL_M::bl_kd:
					kd = kd * 0.5f + tex_map->SampleAt(uv) * 0.5f;
					break;

				default:
					break;
				}
			}
			float d = glm::distance(pl->position, position);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = kd * pl->intensity *
				glm::max(glm::dot(normal, l_vec), 0.0f) / (d * d);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * pl->intensity *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), material->m_shininess) / (d * d);
			return specular + diffuse;
		}
	};
}
