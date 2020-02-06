#pragma once

#include <string>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	struct Material
	{
		std::string shader_var_name;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;

		Material(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
			:shader_var_name(name), ambient(ambi), diffuse(diff), specular(spec), shininess(shin) {}

		Material()
			:shader_var_name("u_Material"), ambient(glm::vec3(1.0f, 1.0f, 1.0f)),
			diffuse(glm::vec3(1.0f, 1.0f, 1.0f)), specular(glm::vec3(1.0f)), shininess(60.0f)
		{}

	};
}