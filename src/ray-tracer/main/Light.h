#pragma once

#include <string>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	struct DirectionalLight {
		std::string shader_var_name = "u_DirLights";

		glm::vec3 direction;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		DirectionalLight(glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, std::string name = "u_DirLights")
			: direction(dir), ambient(amb), diffuse(diff), specular(spec), shader_var_name(name)
		{}
	};

}