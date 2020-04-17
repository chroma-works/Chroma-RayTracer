#pragma once

#include <string>
#include<algorithm> 
#include <ray-tracer/editor/Logger.h>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	enum class MAT_TYPE { none, mirror, dielectric, conductor};
	class Material
	{
	public:

		Material(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin, MAT_TYPE t = MAT_TYPE::none)
			:shader_var_name(name), m_ambient(ambi), m_diffuse(diff), m_specular(spec), m_shininess(shin), type(t) 
		{}

		Material()
			:shader_var_name("u_Material"), m_ambient(glm::vec3(1.0f, 1.0f, 1.0f)),
			m_diffuse(glm::vec3(1.0f, 1.0f, 1.0f)), m_specular(glm::vec3(1.0f)), m_shininess(60.0f), type(MAT_TYPE::none)
		{}

		Material(const Material& mat)
			: shader_var_name(mat.shader_var_name),
			m_ambient(mat.m_ambient),
			m_diffuse(mat.m_diffuse),
			m_specular(mat.m_specular),
			m_shininess(mat.m_shininess),
			m_roughness(mat.m_roughness)
		{
			type = MAT_TYPE::none;
		}

		/*Material& operator=(Material const& mat)
		{
			Material ret;
			ret.shader_var_name = mat.shader_var_name;
			ret.m_ambient = mat.m_ambient;
			ret.m_diffuse = mat.m_diffuse;
			ret.m_specular = mat.m_specular;
			ret.m_shininess = mat.m_shininess;
			ret.m_roughness = mat.m_roughness;

			return ret;
		}*/

		~Material()
		{}

		std::string shader_var_name;
		glm::vec3 m_ambient;
		glm::vec3 m_diffuse;
		glm::vec3 m_specular;
		float m_shininess;
		float m_roughness = 0.0f;
		MAT_TYPE type = MAT_TYPE::none;

		inline float GetFr(float cos_i)
		{
			return NAN;
		}
	};

	class Conductor : public Material
	{
	public:
		Conductor(glm::vec3 _mirror_ref, float _refraction_ind, float _absorption_ind)
			: Material()
		{
			m_mirror_reflec = _mirror_ref;
			m_absorption_ind = _absorption_ind;
			m_refraction_ind = _refraction_ind;
			type = MAT_TYPE::conductor;
		}

		Conductor(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
			:Material(name, ambi, diff, spec, shin, MAT_TYPE::conductor)
		{}

		Conductor(Material mat)
			: Material(mat)
		{
			type = MAT_TYPE::conductor;
		}

		Conductor()
			: Material()
		{
			type = MAT_TYPE::conductor;
		}

		float GetFr(float cos_i)
		{
			float rs = ((m_refraction_ind * m_refraction_ind +
				m_absorption_ind * m_absorption_ind) - 2.0f *
				m_refraction_ind * cos_i - cos_i * cos_i) /
				((m_refraction_ind * m_refraction_ind +
					m_absorption_ind * m_absorption_ind) + 2.0f *
					m_refraction_ind * cos_i - cos_i * cos_i);
			float rp = ((m_refraction_ind * m_refraction_ind +
				m_absorption_ind * m_absorption_ind) *
				cos_i * cos_i - 2.0f *
				m_refraction_ind * cos_i + 1) /
				((m_refraction_ind * m_refraction_ind +
					m_absorption_ind * m_absorption_ind) *
					cos_i * cos_i + 2.0f *
					m_refraction_ind * cos_i + 1);
			return (rs + rp) * 0.5f;
		}

		glm::vec3 m_mirror_reflec = glm::vec3(0.0f);
		float m_refraction_ind = NAN;
		float m_absorption_ind = NAN;
	};

	class Dielectric : public Material
	{
	public:
		Dielectric(glm::vec3 _absorption_coeff, float _refraction_ind)
			: Material()
		{
			m_absorption_coeff = _absorption_coeff;
			m_refraction_ind = _refraction_ind;
			type = MAT_TYPE::conductor;
		}

		Dielectric(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
			:Material(name, ambi, diff, spec, shin, MAT_TYPE::dielectric)
		{}

		Dielectric(Material mat)
			: Material(mat)
		{
			type = MAT_TYPE::dielectric;
		}

		Dielectric()
			: Material()
		{
			type = MAT_TYPE::dielectric;
		}

		float GetFr(float cos_i)
		{
			float ni = 1.0f;
			float nt = m_refraction_ind;
			if (cos_i > 0.0f)
				std::swap(ni, nt);

			cos_i = std::abs(cos_i);


			float sin_i = std::sqrt(std::max(0.0f, 1.0f - cos_i * cos_i));
			float sin_t = ni / nt * sin_i;
			float cos_t = std::sqrt(std::max(0.0f, 1.0f - sin_t * sin_t));

			if (sin_t > 1.0f)
				return 1.0f;

			float r_parl = ((nt * cos_i) - (ni * cos_t)) /
				((nt * cos_i) + (ni * cos_t));
			float r_perp = ((ni * cos_i) - (nt * cos_t)) /
				((ni * cos_i) + (nt * cos_t));
			return (r_parl * r_parl + r_perp * r_perp) * 0.5f;
		}

		glm::vec3 m_absorption_coeff = glm::vec3(0.0f);
		float m_refraction_ind = NAN;
	};

	class Mirror : public Material
	{
	public:
		Mirror(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
			:Material(name, ambi, diff, spec, shin, MAT_TYPE::mirror)
		{}

		Mirror(Material mat)
			: Material(mat)
		{
			type = MAT_TYPE::mirror;
		}

		Mirror()
			: Material()
		{
			type = MAT_TYPE::mirror;
		}

		float GetFr(float cos_i)
		{
			return 1.0f;
		}

		glm::vec3 m_mirror_reflec = glm::vec3(1.0f, 1.0f, 1.0f);
	};
}