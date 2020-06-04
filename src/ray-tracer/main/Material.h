#pragma once

#include <string>
#include<algorithm> 
#include <ray-tracer/editor/Logger.h>
#include <ray-tracer/main/BRDF.h>
#include <thirdparty/glm/glm/glm.hpp>

namespace CHR
{
	enum class MAT_TYPE { none, mirror, dielectric, conductor};
	class Material
	{
	public:

		Material(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float exp, MAT_TYPE t = MAT_TYPE::none)
			:shader_var_name(name), m_ambient(ambi), m_diffuse(diff), m_specular(spec), type(t) 
		{
		}

		Material()
			:shader_var_name("u_Material"), m_ambient(glm::vec3(1.0f, 1.0f, 1.0f)),
			m_diffuse(glm::vec3(1.0f, 1.0f, 1.0f)), m_specular(glm::vec3(1.0f)), type(MAT_TYPE::none)
		{
		}

		Material(const Material& mat)
			: shader_var_name(mat.shader_var_name),
			m_ambient(mat.m_ambient),
			m_diffuse(mat.m_diffuse),
			m_specular(mat.m_specular),
			m_roughness(mat.m_roughness)
		{
			m_brdf = mat.m_brdf;
			type = MAT_TYPE::none;
		}

		inline void Degamma()
		{
			glm::vec3 g = { 2.2, 2.2, 2.2 };
			m_ambient = glm::pow(m_ambient, g);
			m_diffuse = glm::pow(m_diffuse, g);
			m_specular = glm::pow(m_specular, g);
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
		float m_roughness = 0.0f;
		std::shared_ptr<BRDF> m_brdf = std::make_shared<BlinnPhong>();
		
		MAT_TYPE type = MAT_TYPE::none;

		inline float GetFr(float cos_i) const
		{
			return NAN;
		}
		inline glm::vec3 Shade(const glm::vec3 l_vec, const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			glm::vec3 specular = m_brdf->CalculateSpecular(l_vec, e_vec, normal) * m_specular;
			glm::vec3 diffuse = m_brdf->CalculateDiffuse(l_vec, e_vec, normal) * m_diffuse;
			return specular + diffuse;
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

		float GetFr(float cos_i) const
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

		float GetFr(float cos_i) const
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

		float GetFr(float cos_i) const
		{
			return 1.0f;
		}

		glm::vec3 m_mirror_reflec = glm::vec3(1.0f, 1.0f, 1.0f);
	};
}