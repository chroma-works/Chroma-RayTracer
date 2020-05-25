#pragma once

#include <algorithm>
#include <string>

#include <ray-tracer/editor/ImGuiDrawable.h>

#include <thirdparty/glm/glm/common.hpp>
#include <thirdparty/glm/glm/glm.hpp>

#include "Material.h"
#include "Utilities.h"

#define SET_INTENSITY(a,d,s, intensity) void SetIntensity(glm::vec3 inten){intensity = inten; a=s=d=(glm::clamp(inten/1000.0f, 0.0f, 1.0f));}

namespace Chroma
{
	enum class LIGHT_T {point, directional, spot, enviroment, area};
	class Light : public ImGuiDrawable
	{
	public:

		SET_INTENSITY(ambient, diffuse, specular, ls)

		virtual glm::vec3 IlluminationAt(const glm::vec3 isect_pos, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const = 0;

		std::string shader_var_name;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 ls;

		LIGHT_T type;
	};

	class DirectionalLight : public Light {
	public:

		DirectionalLight(glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, std::string name = "u_DirLights")
			: direction(glm::normalize(dir))
		{
			ambient = amb;
			diffuse = diff;
			specular = spec;
			shader_var_name = name;
			ls = { 0,0,0 };
			type = LIGHT_T::directional;
		}

		DirectionalLight(const DirectionalLight& other)
			: direction(other.direction)
		{
			ambient = other.ambient;
			diffuse = other.diffuse;
			specular = other.specular;
			shader_var_name = other.shader_var_name;
			ls = other.ls;
			type = LIGHT_T::directional;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 l_vec = glm::normalize(-direction);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * ls *
				glm::max(glm::dot(isect_normal, l_vec), 0.0f);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * ls *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_normal))), material->m_shininess);
			return specular + diffuse;
		}
		void DrawUI()
		{
			ImGui::Text("DirectionalLight");
			ImGui::Separator();
			ImGui::Text("Transform");
			if (ImGui::Button("D##1"))direction = glm::vec3(1.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(direction.x), 0.05f, 0, 0, "%.3f");
			direction = glm::normalize(direction);

			ImGui::Separator();

			ImGui::Text("Light");
			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);
			ImGui::ColorEdit3("Ambient Color", &ambient.x);
			ImGui::ColorEdit3("Diffuse Color", &diffuse.x);
			ImGui::ColorEdit3("Specular Color", &specular.x);

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = ls;
			if (ImGui::DragFloat3("Radiance", &tmp.x, 1.0f, 0.0f, 1000.0f))
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
			ls = { 0,0,0 };
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
			ls = other.ls;
			type = LIGHT_T::point;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 l_vec = glm::normalize(position - isect_position);
			float d = glm::distance(position, isect_position);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * ls *
				glm::max(glm::dot(isect_normal, l_vec), 0.0f) / (d * d);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * ls *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_normal))), material->m_shininess) / (d * d);
			return specular + diffuse;
		}

		void DrawUI()
		{
			ImGui::Text("PointLight");
			ImGui::Separator();
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
			glm::vec3 tmp = ls;
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

	class SpotLight : public Light {
	public:
		//std::string shader_var_name = "u_SpotLights";

		glm::vec3 position;
		glm::vec3 direction;
		float fall_off;
		float cut_off;

		float constant;
		float linear;
		float quadratic;

		/*glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 intensity = { 0,0,0 };

		SET_INTENSITY(ambient, diffuse, specular, intensity)*/

		SpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, float cons = 1.0f, float lin = 0.01f, float quad = 0.0001f,
			float cut = 0.976296f, float outerCut = 0.963630f, std::string name = "u_SpotLights")
			: position(pos), direction(dir), constant(cons), linear(lin), quadratic(quad),
			fall_off(cut), cut_off(outerCut)
		{
			ambient = amb;
			diffuse = diff;
			specular = spec;
			shader_var_name = name;
			ls = { 0,0,0 };
			type = LIGHT_T::spot;
		}

		SpotLight(const SpotLight& other)
			: position(other.position), direction(other.direction),
			constant(other.constant), linear(other.linear),
			quadratic(other.quadratic), fall_off(other.fall_off),
			cut_off(other.cut_off)
		{
			ambient = other.ambient;
			diffuse = other.diffuse;
			specular = other.specular;
			shader_var_name = other.shader_var_name;
			ls = other.ls;
			type = LIGHT_T::spot;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 intensity = ls;
			glm::vec3 l_vec = glm::normalize(position - isect_position);
			float theta = acos(glm::dot(l_vec, normalize(-direction)));
			// spotlight intensity
			float epsilon = fall_off/2 - cut_off/2;
			intensity *= pow(glm::clamp((theta - cut_off/2) / epsilon, 0.0f, 1.0f), 4);
			float d = glm::distance(position, isect_position);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * intensity *
				glm::max(glm::dot(isect_normal, l_vec), 0.0f) / (d * d);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * intensity *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_normal))), material->m_shininess) / (d * d);
			return specular + diffuse;
		}

		void DrawUI()
		{
			ImGui::Text("SpotLight");
			ImGui::Separator();
			ImGui::Text("Transform");
			if (ImGui::Button("P##1"))position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##5", &(position.x), 0.05f, 0, 0, "%.3f");
			if (ImGui::Button("D##2"))direction = glm::vec3(1.0f,0.0f,0.0f);
			ImGui::SameLine();
			ImGui::DragFloat3("##6", &(direction.x), 0.05f, 0, 0, "%.3f");
			direction = glm::normalize(direction);

			ImGui::Separator();

			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::Text("Light");
			ImGui::ColorEdit3("Ambient Color", &ambient.x);
			ImGui::ColorEdit3("Diffuse Color", &diffuse.x);
			ImGui::ColorEdit3("Specular Color", &specular.x);

			ImGui::Separator();

			if (ImGui::Button("Fall-off##3"))fall_off = 0.1;
			ImGui::SameLine();
			ImGui::DragFloat("##7", &(fall_off), 0.05f, 0.00001, 3600, "%.3f");
			if (ImGui::Button("Cut-off##4"))cut_off = 0.5;
			ImGui::SameLine();
			ImGui::DragFloat("##8", &(cut_off), 0.05f, fall_off, 360, "%.3f");

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = ls;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				SetIntensity(tmp);
				/*m_scene->m_spot_lights[selected_name]->ambient =
					m_scene->m_spot_lights[selected_name]->diffuse =
					m_scene->m_spot_lights[selected_name]->specular = tmp;*/
			}
		}
	};

	class AreaLight : public Light
	{
	public:
		glm::vec3 position;
		glm::vec3 normal;
		float size = 1.0f;


		AreaLight(glm::vec3 pos, glm::vec3 norm, float _size, std::string name = "Non_renderable")
			: position(pos), normal(norm), size(_size)
		{
			ambient = {0,0,0};
			diffuse = { 0,0,0 };
			specular = { 0,0,0 };
			shader_var_name = name;
			ls = { 0,0,0 };
			type = LIGHT_T::area;
		}

		AreaLight(const AreaLight& other)
			: position(other.position), normal(other.normal), size(other.size)
		{
			ambient = other.ambient;
			diffuse = other.diffuse;
			specular = other.specular;
			shader_var_name = other.shader_var_name;
			ls = other.ls;
			type = LIGHT_T::area;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 u, v;
			Utils::CreateOrthonormBasis(normal, u, v);
			glm::vec3 sample_pos = position + (Utils::RandFloat(-size / 2.0f, size / 2.0f) * u +
				Utils::RandFloat(-size / 2.0f, size / 2.0f) * v);
 
			glm::vec3 l_vec = glm::normalize(sample_pos - isect_position);
			float d = glm::distance(isect_position, sample_pos);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * ls *
				glm::max(glm::dot(isect_normal, l_vec), 0.0f) / (d * d);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * ls *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_normal))), material->m_shininess) / (d * d);

			float cos_t = abs(glm::dot(l_vec,glm::normalize(normal)));
			return (specular + diffuse) * cos_t * size *size ;
		}

		void DrawUI()
		{
			ImGui::Text("AreaLight(No preview render)");
			ImGui::Separator();
			ImGui::Text("Transform");
			if (ImGui::Button("P##1"))position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(position.x), 0.05f, 0, 0, "%.3f");
			if (ImGui::Button("D##2"))normal = glm::vec3(1.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::DragFloat3("##5", &(normal.x), 0.05f, 0, 0, "%.3f");
			normal = glm::normalize(normal);
			if (ImGui::Button("S##3"))size = 0.0f;
			ImGui::SameLine();
			ImGui::DragFloat("##6", &size, 0.05f, 0.0001, 0, "%.3f");

			ImGui::Separator();

			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::Text("Light");

			ImGui::Separator();
			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = ls;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				SetIntensity(tmp);
				/*m_scene->m_spot_lights[selected_name]->ambient =
					m_scene->m_spot_lights[selected_name]->diffuse =
					m_scene->m_spot_lights[selected_name]->specular = tmp;*/
			}
		}
	};

}