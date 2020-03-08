#pragma once
#include <thirdparty/glm/glm/glm.hpp>
#include <ray-tracer/main/Material.h>

namespace Chroma
{

	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;

		float intersect_eps = 0.0f;

		Ray(glm::vec3 orig = glm::vec3(0, 0, 0), glm::vec3 dir = glm::vec3(0, 0, 0))
			:origin(orig), direction(dir)
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

		bool hit = false;
	};
}
