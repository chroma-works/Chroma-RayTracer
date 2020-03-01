#pragma once

#include <string>
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

				float FR() { return 0.4f; };
			}dielectric_coeffs;

			struct ConductorCoeffs
			{
				glm::vec3 mirror_reflec = glm::vec3(0.0f);
				float refraction_ind = NAN;
				float absorptionI_ind = NAN; 

				ConductorCoeffs() {};
				float FR() { return 0.5f; };
			}conductor_coeffs;

			FrensnelCoeffs() {};

		}f_coeff;

		float GetFr(float cos_theta)
		{
			if (type == MAT_TYPE::conductor)
			{
				float rs = ((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
					f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) - 2.0f *
					f_coeff.conductor_coeffs.refraction_ind * cos_theta - cos_theta * cos_theta) /
					((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
						f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) + 2.0f *
						f_coeff.conductor_coeffs.refraction_ind * cos_theta - cos_theta * cos_theta);
				float rp = ((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
					f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) *
					cos_theta * cos_theta - 2.0f *
					f_coeff.conductor_coeffs.refraction_ind * cos_theta + 1) /
					((f_coeff.conductor_coeffs.refraction_ind * f_coeff.conductor_coeffs.refraction_ind +
						f_coeff.conductor_coeffs.absorptionI_ind * f_coeff.conductor_coeffs.absorptionI_ind) *
						cos_theta * cos_theta + 2.0f *
						f_coeff.conductor_coeffs.refraction_ind * cos_theta + 1);
				return (rs + rp) * 0.5f;
			}
			else if (type == MAT_TYPE::dielectric)
			{
				return 0.5f;
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