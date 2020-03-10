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
			:shader_var_name(name), ambient(ambi), diffuse(diff), specular(spec), shininess(shin), type(t) 
		{}

		Material()
			:shader_var_name("u_Material"), ambient(glm::vec3(1.0f, 1.0f, 1.0f)),
			diffuse(glm::vec3(1.0f, 1.0f, 1.0f)), specular(glm::vec3(1.0f)), shininess(60.0f), type(MAT_TYPE::none)
		{}

		/*Material(Material&&) = delete;
		Material(Material const&) = delete;
		Material& operator=(Material) = delete;*/

		std::string shader_var_name;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
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
			mirror_reflec = _mirror_ref;
			absorption_ind = _absorption_ind;
			refraction_ind = _refraction_ind;
			type = MAT_TYPE::conductor;
		}

		Conductor(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
			:Material(name, ambi, diff, spec, shin, MAT_TYPE::conductor)
		{}

		Conductor()
			: Material()
		{
			type = MAT_TYPE::conductor;
		}

		float GetFr(float cos_i)
		{
			float rs = ((refraction_ind * refraction_ind +
				absorption_ind * absorption_ind) - 2.0f *
				refraction_ind * cos_i - cos_i * cos_i) /
				((refraction_ind * refraction_ind +
					absorption_ind * absorption_ind) + 2.0f *
					refraction_ind * cos_i - cos_i * cos_i);
			float rp = ((refraction_ind * refraction_ind +
				absorption_ind * absorption_ind) *
				cos_i * cos_i - 2.0f *
				refraction_ind * cos_i + 1) /
				((refraction_ind * refraction_ind +
					absorption_ind * absorption_ind) *
					cos_i * cos_i + 2.0f *
					refraction_ind * cos_i + 1);
			return (rs + rp) * 0.5f;
		}

		glm::vec3 mirror_reflec = glm::vec3(0.0f);
		float refraction_ind = NAN;
		float absorption_ind = NAN;

	};

	class Dielectric : public Material
	{
	public:
		Dielectric(glm::vec3 _absorption_coeff, float _refraction_ind)
			: Material()
		{
			absorption_coeff = _absorption_coeff;
			refraction_ind = _refraction_ind;
			type = MAT_TYPE::conductor;
		}

		Dielectric(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
			:Material(name, ambi, diff, spec, shin, MAT_TYPE::dielectric)
		{}

		Dielectric()
			: Material()
		{
			type = MAT_TYPE::dielectric;
		}

		float GetFr(float cos_i)
		{
			float ni = 1.0f;
			float nt = refraction_ind;
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

		glm::vec3 absorption_coeff = glm::vec3(0.0f);
		float refraction_ind = NAN;
	};

	class Mirror : public Material
	{
	public:
		Mirror(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
			:Material(name, ambi, diff, spec, shin, MAT_TYPE::mirror)
		{}

		Mirror()
			: Material()
		{
			type = MAT_TYPE::mirror;
		}

		float GetFr(float cos_i)
		{
			return 1.0f;
		}

		glm::vec3 mirror_reflec = glm::vec3(1.0f, 1.0f, 1.0f);
	};
}