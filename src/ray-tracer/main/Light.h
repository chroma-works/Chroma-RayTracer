#pragma once

#include <algorithm>
#include <string>

#include <ray-tracer/editor/ImGuiDrawable.h>

#include <thirdparty/glm/glm/common.hpp>
#include <thirdparty/glm/glm/glm.hpp>

#include "Texture.h"
#include "Material.h"
#include "Utilities.h"
//#include <ray-tracer\main\ImageTextureMap.h>

#define SET_INTENSITY(a,d,s, intensity) void SetIntensity(glm::vec3 inten){intensity = inten; a=s=d=(glm::clamp(inten/1000.0f, 0.0f, 1.0f));}

namespace Chroma
{
	enum class LIGHT_T {point, directional, spot, environment, area};
	class Light : public ImGuiDrawable
	{
	public:

		SET_INTENSITY(m_ambient, m_diffuse, m_specular, m_inten)

		virtual glm::vec3 IlluminationAt(const glm::vec3 isect_pos, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const = 0;

		std::string m_shader_var_name;
		glm::vec3 m_ambient;
		glm::vec3 m_diffuse;
		glm::vec3 m_specular;

		glm::vec3 m_inten;

		LIGHT_T m_type;
	};

	class DirectionalLight : public Light {
	public:

		DirectionalLight(glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, std::string name = "u_DirLights")
			: m_direction(glm::normalize(dir))
		{
			m_ambient = amb;
			m_diffuse = diff;
			m_specular = spec;
			m_shader_var_name = name;
			m_inten = { 0,0,0 };
			m_type = LIGHT_T::directional;
		}

		DirectionalLight(const DirectionalLight& other)
			: m_direction(other.m_direction)
		{
			m_ambient = other.m_ambient;
			m_diffuse = other.m_diffuse;
			m_specular = other.m_specular;
			m_shader_var_name = other.m_shader_var_name;
			m_inten = other.m_inten;
			m_type = LIGHT_T::directional;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 l_vec = glm::normalize(-m_direction);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * m_inten *
				glm::max(glm::dot(isect_normal, l_vec), 0.0f);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * m_inten *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_normal))), material->m_shininess);
			return specular + diffuse;
		}
		void DrawUI()
		{
			ImGui::Text("DirectionalLight");
			ImGui::Separator();
			ImGui::Text("Transform");
			if (ImGui::Button("D##1"))m_direction = glm::vec3(1.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(m_direction.x), 0.05f, 0, 0, "%.3f");
			m_direction = glm::normalize(m_direction);

			ImGui::Separator();

			ImGui::Text("Light");
			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);
			ImGui::ColorEdit3("Ambient Color", &m_ambient.x);
			ImGui::ColorEdit3("Diffuse Color", &m_diffuse.x);
			ImGui::ColorEdit3("Specular Color", &m_specular.x);

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = m_inten;
			if (ImGui::DragFloat3("Radiance", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				SetIntensity(tmp);
				/*glm::vec3 tmp = m_scene->m_dir_lights[selected_name]->intensity / 1000.0f;
				m_scene->m_dir_lights[selected_name]->ambient =
					m_scene->m_dir_lights[selected_name]->diffuse =
					m_scene->m_dir_lights[selected_name]->specular = tmp;*/
			}
		}

		glm::vec3 m_direction;
	};

	class PointLight : public Light{
	public:
		glm::vec3 m_position;

		float m_constant;
		float m_linear;
		float m_quadratic;

		PointLight(glm::vec3 pos, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, float cons = 1.0f, float lin = 0.01f, float quad = 0.0001f,
			std::string name = "u_PointLights")
			: m_position(pos), m_constant(cons), m_linear(lin), m_quadratic(quad)
		{
			m_ambient = amb;
			m_diffuse = diff;
			m_specular = spec;
			m_shader_var_name = name;
			m_inten = { 0,0,0 };
			m_type = LIGHT_T::point;
		}

		PointLight(const PointLight& other)
			: m_position(other.m_position),m_constant(other.m_constant),
			m_linear(other.m_linear), m_quadratic(other.m_quadratic)
		{
			m_ambient = other.m_ambient;
			m_diffuse = other.m_diffuse;
			m_specular = other.m_specular;
			m_shader_var_name = other.m_shader_var_name;
			m_inten = other.m_inten;
			m_type = LIGHT_T::point;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 l_vec = glm::normalize(m_position - isect_position);
			float d = glm::distance(m_position, isect_position);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * m_inten *
				glm::max(glm::dot(isect_normal, l_vec), 0.0f) / (d * d);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * m_inten *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_normal))), material->m_shininess) / (d * d);
			return specular + diffuse;
		}

		void DrawUI()
		{
			ImGui::Text("PointLight");
			ImGui::Separator();
			ImGui::Text("Transform");
			if (ImGui::Button("P##1"))m_position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(m_position.x), 0.05f, 0, 0, "%.3f");

			ImGui::Separator();

			ImGui::Text("Light");
			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::ColorEdit3("Ambient Color", &m_ambient.x, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit3("Diffuse Color", &m_diffuse.x, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit3("Specular Color", &m_specular.x, ImGuiColorEditFlags_Float);


			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = m_inten;
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

		glm::vec3 m_position;
		glm::vec3 m_direction;
		float m_fall_off;
		float m_cut_off;

		float m_constant;
		float m_linear;
		float m_quadratic;

		/*glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		glm::vec3 intensity = { 0,0,0 };

		SET_INTENSITY(ambient, diffuse, specular, intensity)*/

		SpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 amb,
			glm::vec3 diff, glm::vec3 spec, float cons = 1.0f, float lin = 0.01f, float quad = 0.0001f,
			float cut = 0.976296f, float outerCut = 0.963630f, std::string name = "u_SpotLights")
			: m_position(pos), m_direction(dir), m_constant(cons), m_linear(lin), m_quadratic(quad),
			m_fall_off(cut), m_cut_off(outerCut)
		{
			m_ambient = amb;
			m_diffuse = diff;
			m_specular = spec;
			m_shader_var_name = name;
			m_inten = { 0,0,0 };
			m_type = LIGHT_T::spot;
		}

		SpotLight(const SpotLight& other)
			: m_position(other.m_position), m_direction(other.m_direction),
			m_constant(other.m_constant), m_linear(other.m_linear),
			m_quadratic(other.m_quadratic), m_fall_off(other.m_fall_off),
			m_cut_off(other.m_cut_off)
		{
			m_ambient = other.m_ambient;
			m_diffuse = other.m_diffuse;
			m_specular = other.m_specular;
			m_shader_var_name = other.m_shader_var_name;
			m_inten = other.m_inten;
			m_type = LIGHT_T::spot;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 intensity = m_inten;
			glm::vec3 l_vec = glm::normalize(m_position - isect_position);
			float theta = acos(glm::dot(l_vec, normalize(-m_direction)));
			// spotlight intensity
			float epsilon = m_fall_off/2 - m_cut_off/2;
			intensity *= pow(glm::clamp((theta - m_cut_off/2) / epsilon, 0.0f, 1.0f), 4);
			float d = glm::distance(m_position, isect_position);
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
			if (ImGui::Button("P##1"))m_position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##5", &(m_position.x), 0.05f, 0, 0, "%.3f");
			if (ImGui::Button("D##2"))m_direction = glm::vec3(1.0f,0.0f,0.0f);
			ImGui::SameLine();
			ImGui::DragFloat3("##6", &(m_direction.x), 0.05f, 0, 0, "%.3f");
			m_direction = glm::normalize(m_direction);

			ImGui::Separator();

			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::Text("Light");
			ImGui::ColorEdit3("Ambient Color", &m_ambient.x);
			ImGui::ColorEdit3("Diffuse Color", &m_diffuse.x);
			ImGui::ColorEdit3("Specular Color", &m_specular.x);

			ImGui::Separator();

			if (ImGui::Button("Fall-off##3"))m_fall_off = 0.1;
			ImGui::SameLine();
			ImGui::DragFloat("##7", &(m_fall_off), 0.05f, 0.00001, 3600, "%.3f");
			if (ImGui::Button("Cut-off##4"))m_cut_off = 0.5;
			ImGui::SameLine();
			ImGui::DragFloat("##8", &(m_cut_off), 0.05f, m_fall_off, 360, "%.3f");

			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = m_inten;
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
		glm::vec3 m_position;
		glm::vec3 m_normal;
		float m_size = 1.0f;


		AreaLight(glm::vec3 pos, glm::vec3 norm, float _size, std::string name = "Non_renderable")
			: m_position(pos), m_normal(norm), m_size(_size)
		{
			m_ambient = {0,0,0};
			m_diffuse = { 0,0,0 };
			m_specular = { 0,0,0 };
			m_shader_var_name = name;
			m_inten = { 0,0,0 };
			m_type = LIGHT_T::area;
		}

		AreaLight(const AreaLight& other)
			: m_position(other.m_position), m_normal(other.m_normal), m_size(other.m_size)
		{
			m_ambient = other.m_ambient;
			m_diffuse = other.m_diffuse;
			m_specular = other.m_specular;
			m_shader_var_name = other.m_shader_var_name;
			m_inten = other.m_inten;
			m_type = LIGHT_T::area;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			glm::vec3 u, v;
			Utils::CreateOrthonormBasis(m_normal, u, v);
			glm::vec3 sample_pos = m_position + (Utils::RandFloat(-m_size / 2.0f, m_size / 2.0f) * u +
				Utils::RandFloat(-m_size/2.0f, m_size/2.0f) * v);
 
			glm::vec3 l_vec = glm::normalize(sample_pos - isect_position);
			float d = glm::distance(isect_position, sample_pos);
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material->m_diffuse * m_inten *
				glm::max(glm::dot(isect_normal, l_vec), 0.0f) / (d * d);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material->m_specular * m_inten *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_normal))), material->m_shininess) / (d * d);

			float cos_t = abs(glm::dot(l_vec,glm::normalize(m_normal)));
			return (specular + diffuse) * cos_t * m_size *m_size ;
		}

		void DrawUI()
		{
			ImGui::Text("AreaLight(No preview render)");
			ImGui::Separator();
			ImGui::Text("Transform");
			if (ImGui::Button("P##1"))m_position = glm::vec3();
			ImGui::SameLine();
			ImGui::DragFloat3("##4", &(m_position.x), 0.05f, 0, 0, "%.3f");
			if (ImGui::Button("D##2"))m_normal = glm::vec3(1.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::DragFloat3("##5", &(m_normal.x), 0.05f, 0, 0, "%.3f");
			m_normal = glm::normalize(m_normal);
			if (ImGui::Button("S##3"))m_size = 0.0f;
			ImGui::SameLine();
			ImGui::DragFloat("##6", &m_size, 0.05f, 0.0001, 0, "%.3f");

			ImGui::Separator();

			ImGui::CollapsingHeader("Phong Lighting(Editor)", ImGuiTreeNodeFlags_Leaf);

			ImGui::Text("Light");

			ImGui::Separator();
			ImGui::CollapsingHeader("RT colors", ImGuiTreeNodeFlags_Leaf);
			glm::vec3 tmp = m_inten;
			if (ImGui::DragFloat3("Intensity", &tmp.x, 1.0f, 0.0f, 1000.0f))
			{
				SetIntensity(tmp);
				/*m_scene->m_spot_lights[selected_name]->ambient =
					m_scene->m_spot_lights[selected_name]->diffuse =
					m_scene->m_spot_lights[selected_name]->specular = tmp;*/
			}
		}
	};

	class EnvironmentLight : public Light {
	public:

		EnvironmentLight(std::shared_ptr<Texture> img, std::string name = "Non_renderable")
			: m_tex(img)
		{
			m_ambient = { 0,0,0 };
			m_diffuse = { 0,0,0 };
			m_specular = { 0,0,0 };
			m_shader_var_name = name;
			m_inten = { 0,0,0 };
			m_type = LIGHT_T::environment;
		}

		EnvironmentLight(const EnvironmentLight& other)
			: m_tex(other.m_tex)
		{
			m_ambient = other.m_ambient;
			m_diffuse = other.m_diffuse;
			m_specular = other.m_specular;
			m_shader_var_name = other.m_shader_var_name;
			m_inten = other.m_inten;
			m_type = LIGHT_T::environment;
		}

		glm::vec3 IlluminationAt(const glm::vec3 isect_position, const glm::vec3 isect_normal,
			const glm::vec3 e_vec, const Material* material) const
		{
			//Randomly sample a direction. Random rejection sampling
			glm::vec3 direction;
			while (true)
			{
				direction = { Utils::RandFloat(-1,1), Utils::RandFloat(-1,1), Utils::RandFloat(-1,1) };

				bool valid_direction = glm::length(direction) < 1.0f &&
					glm::dot(direction, isect_normal) > 0.0f;
				if (valid_direction)
					break;
			}
			direction = glm::normalize(direction);
			return { 100,100,100 };
		}
		void DrawUI()
		{
			ImGui::Text("Environment Light");
			ImGui::Separator();
			char* a = new char[m_tex->GetFilePath().size()];
			strcpy(a, m_tex->GetFilePath().c_str());
			ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
			if (ImGui::InputText("Radiance Map", a, 128, flags))
			{
				if (std::string(a).compare(m_tex->GetFilePath()) != 0)
				{
					m_tex = std::make_shared<Texture>(std::string(a));
				}
			}
			delete[] a;
		}

		std::shared_ptr<Texture> m_tex;
	};

}