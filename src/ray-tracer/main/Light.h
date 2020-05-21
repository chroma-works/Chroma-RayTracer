#pragma once

#include <string>
#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/imgui/imgui.h>
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"

#include "Material.h"

#define SET_INTENSITY(a,d,s, intensity) void SetIntensity(glm::vec3 inten){intensity = inten; a=s=d=(glm::clamp(inten/1000.0f, 0.0f, 1.0f));}

namespace Chroma
{
	enum class LIGHT_T {point, directional, spot, enviroment, area};
	class Light
	{
	public:

		SET_INTENSITY(ambient, diffuse, specular, intensity)

		virtual glm::vec3 CalculateRadiance(const glm::vec3 pos, const glm::vec3 normal,
			const glm::vec3 e_vec, const Material* material) const = 0;

		virtual void DrawUI() = 0;

		std::string shader_var_name;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 intensity;

		LIGHT_T type;
	};

	class DirectionalLight : public Light {
	public:

		DirectionalLight(glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, std::string name = "u_DirLights")
			: direction(dir)
		{
			ambient = amb;
			diffuse = diff;
			specular = spec;
			shader_var_name = name;
			intensity = { 0,0,0 };
			type = LIGHT_T::directional;
		}

		DirectionalLight(const DirectionalLight& other)
			: direction(other.direction)
		{
			ambient = other.ambient;
			diffuse = other.diffuse;
			specular = other.specular;
			shader_var_name = other.shader_var_name;
			intensity = other.intensity;
			type = LIGHT_T::directional;
		}

		glm::vec3 CalculateRadiance(const glm::vec3 pos, const glm::vec3 normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * intensity *
				glm::max(glm::dot(normal, direction), 0.0f);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + direction) / glm::length(e_vec + direction));
			glm::vec3 specular = material->m_specular * intensity *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), material->m_shininess);
			return specular + diffuse;
		}
		void DrawUI()
		{
			ImGui::Text("Transform");
			if (ImGui::Button("D##1"))direction = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(direction.x), 0.05f, 0, 0, "%.3f");

			ImGui::Separator();

			ImGui::Text("Light");
			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);
			ImGui::ColorEdit3("Ambient Color", &ambient.x);
			ImGui::ColorEdit3("Diffuse Color", &diffuse.x);
			ImGui::ColorEdit3("Specular Color", &specular.x);

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = intensity;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				SetIntensity(tmp);
				/*glm::vec3 tmp = m_scene->m_dir_lights[selected_name]->intensity / 1000.0f;
				m_scene->m_dir_lights[selected_name]->ambient =
					m_scene->m_dir_lights[selected_name]->diffuse =
					m_scene->m_dir_lights[selected_name]->specular = tmp;*/
			}
		}

		glm::vec3 direction;
	};

	class PointLight : public Light{
	public:
		glm::vec3 position;

		float constant;
		float linear;
		float quadratic;

		PointLight(glm::vec3 pos, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, float cons = 1.0f, float lin = 0.01f, float quad = 0.0001f,
			std::string name = "u_PointLights")
			: position(pos), constant(cons), linear(lin), quadratic(quad)
		{
			ambient = amb;
			diffuse = diff;
			specular = spec;
			shader_var_name = name;
			intensity = { 0,0,0 };
			type = LIGHT_T::point;
		}

		PointLight(const PointLight& other)
			: position(other.position),constant(other.constant),
			linear(other.linear), quadratic(other.quadratic)
		{
			ambient = other.ambient;
			diffuse = other.diffuse;
			specular = other.specular;
			shader_var_name = other.shader_var_name;
			intensity = other.intensity;
			type = LIGHT_T::point;
		}

		glm::vec3 CalculateRadiance(const glm::vec3 pos, const glm::vec3 normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 l_vec = glm::normalize(position - pos);
			float d = glm::distance(position, pos);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * intensity *
				glm::max(glm::dot(normal, l_vec), 0.0f) / (d * d);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * intensity *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), material->m_shininess) / (d * d);
			return specular + diffuse;
		}

		void DrawUI()
		{
			ImGui::Text("Transform");
			if (ImGui::Button("P##1"))position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(position.x), 0.05f, 0, 0, "%.3f");

			ImGui::Separator();

			ImGui::Text("Light");
			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::ColorEdit3("Ambient Color", &ambient.x, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit3("Diffuse Color", &diffuse.x, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit3("Specular Color", &specular.x, ImGuiColorEditFlags_Float);


			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = intensity;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				SetIntensity(tmp);
				/*glm::vec3 tmp = m_scene->m_point_lights[selected_name]->intensity / 1000.0f;
				m_scene->m_point_lights[selected_name]->ambient =
					m_scene->m_point_lights[selected_name]->diffuse =
					m_scene->m_point_lights[selected_name]->specular = tmp;*/
			}
		}
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