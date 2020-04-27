#pragma once
#include <thirdparty/glm/glm/glm.hpp>
#include "Material.h"
#include "TextureMap.h"

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
	};
}
