#pragma once

#include <string>
#include<algorithm> 
#include <ray-tracer/editor/Logger.h>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	enum class MAT_TYPE { none, mirror, dielectric, conductor};
	struct Material
	{
		std::string shader_var_name;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
		MAT_TYPE type = MAT_TYPE::none;

		union FrensnelCoeffs
		{
			struct DielectricCoeffs
			{
				glm::vec3 absorption_coeff = glm::vec3(0.0f);
				float refraction_ind = NAN;

				DielectricCoeffs() {};
			}dielectric_coeffs;

			struct ConductorCoeffs
			{
				glm::vec3 mirror_reflec = glm::vec3(0.0f);
				float refraction_ind = NAN;
				float absorptionI_ind = NAN; 

				ConductorCoeffs() {};
			}conductor_coeffs;

			FrensnelCoeffs() {};

		}f_coeff;

		float GetFr(float cos_i)
		{
			if (type == MAT_TYPE::conductor)
			{
				float rs = ((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
					f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) - 2.0f *
					f_coeff.conductor_coeffs.refraction_ind * cos_i - cos_i * cos_i) /
					((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
						f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) + 2.0f *
						f_coeff.conductor_coeffs.refraction_ind * cos_i - cos_i * cos_i);
				float rp = ((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
					f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) *
					cos_i * cos_i - 2.0f *
					f_coeff.conductor_coeffs.refraction_ind * cos_i + 1) /
					((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
						f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) *
						cos_i * cos_i + 2.0f *
						f_coeff.conductor_coeffs.refraction_ind * cos_i + 1);
				return (rs + rp) * 0.5f;
			}
			else if (type == MAT_TYPE::dielectric)
			{
				float ni = 1.0f;
				float nt = f_coeff.dielectric_coeffs.refraction_ind;
				if( cos_i > 0.0f)
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
			else if (type == MAT_TYPE::mirror)
				return 1.0f;
			else
				return NAN;
		}

		Material(std::string name, glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin, MAT_TYPE t = MAT_TYPE::none)
			:shader_var_name(name), ambient(ambi), diffuse(diff), specular(spec), shininess(shin), type(t) {}

		Material()
			:shader_var_name("u_Material"), ambient(glm::vec3(1.0f, 1.0f, 1.0f)),
			diffuse(glm::vec3(1.0f, 1.0f, 1.0f)), specular(glm::vec3(1.0f)), shininess(60.0f),type( MAT_TYPE::none)
		{}

	};
}