#pragma once

#include <string>
#include <thirdparty/glm/glm/glm.hpp>

#define SET_INTENSITY(a,d,s, intensity) void SetIntensity(glm::vec3 inten){intensity = inten; a=s=d=(glm::clamp(inten/1000.0f, 0.0f, 1.0f));}

namespace Chroma
{
	/*inline static void SetIntensity<class l_type>(glm::vec3 intensity)
	{
		
	}*/
	struct DirectionalLight {
		std::string shader_var_name = "u_DirLights";

		glm::vec3 direction;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 intensity;

		SET_INTENSITY(ambient, diffuse, specular, intensity)

		DirectionalLight(glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, std::string name = "u_DirLights")
			: direction(dir), ambient(amb), diffuse(diff), specular(spec), shader_var_name(name), intensity({ 0.0f, 0.0f, 0.0f })
		{}
	};

	struct PointLight {
		std::string shader_var_name = "u_PointLights";

		glm::vec3 position;
		//this might need direction here

		float constant;
		float linear;
		float quadratic;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 intensity;

		SET_INTENSITY(ambient, diffuse, specular, intensity)

		PointLight(glm::vec3 pos, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, float cons = 1.0f, float lin = 0.01f, float quad = 0.0001f,
			std::string name = "u_PointLights")
			: position(pos), ambient(amb), diffuse(diff), specular(spec), constant(cons), linear(lin), quadratic(quad),
			shader_var_name(name), intensity({0.0f, 0.0f, 0.0f})
		{}

		PointLight(const PointLight& other)
			: position(other.position), ambient(other.ambient), diffuse(other.diffuse), specular(other.specular), constant(other.constant), linear(other.linear), quadratic(other.quadratic),
			shader_var_name(other.shader_var_name), intensity(other.intensity)
		{}
	};

	struct SpotLight {
		std::string shader_var_name = "u_SpotLights";

		glm::vec3 position;
		glm::vec3 direction;
		float cutOff;
		float outerCutOff;

		float constant;
		float linear;
		float quadratic;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 intensity = { 0,0,0 };

		SET_INTENSITY(ambient, diffuse, specular, intensity)

		SpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, float cons = 1.0f, float lin = 0.01f, float quad = 0.0001f,
			float cut = 0.976296f, float outerCut = 0.963630f, std::string name = "u_SpotLights")
			: position(pos), direction(dir), ambient(amb), diffuse(diff), specular(spec), constant(cons), linear(lin), quadratic(quad),
			cutOff(cut), outerCutOff(outerCut), shader_var_name(name), intensity({ 0.0f, 0.0f, 0.0f })
		{}
	};

}