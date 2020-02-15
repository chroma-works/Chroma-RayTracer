#pragma once
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{

	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;

		Ray(glm::vec3 orig = glm::vec3(0, 0, 0), glm::vec3 dir = glm::vec3(0, 0, 0))
			:origin(orig), direction(dir)
		{}
	};
}
