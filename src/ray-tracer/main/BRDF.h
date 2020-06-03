#pragma once

namespace CHR
{

	//class CHR::Material;
	class BRDF
	{
	public:
		BRDF(float exp = 50.0f, bool norm = false)
			:m_exponent(exp), m_normalized(norm)
		{}
		float m_exponent = 50;
		bool m_normalized = false;

	private:
		friend class Material;
		virtual glm::vec3 Shade(const glm::vec3 l_vec, const glm::vec3 e_vec, 
			const glm::vec3 radiance, const glm::vec3 normal, 
			const glm::vec3 material_diffuse, const glm::vec3 material_specular) const = 0;
	};

	class BlinnPhong : public BRDF
	{
	public:
		BlinnPhong(float exp = 50.0f, bool norm = false)
			:BRDF(exp, norm)
		{}

		inline glm::vec3 Shade(const glm::vec3 l_vec, const glm::vec3 e_vec, 
			const glm::vec3 radiance, const glm::vec3 normal, 
			const glm::vec3 material_diffuse, const glm::vec3 material_specular) const
		{
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material_diffuse * radiance *
				glm::max(glm::dot(normal, l_vec), 0.0f);
			//Ks* I * max(0, h . n)^s / d^2
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 specular = material_specular * radiance *
				glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), m_exponent);

			return specular + diffuse;
		}
	};

	class Phong : public BRDF
	{
	public:
		Phong(float exp = 50.0f, bool norm = false)
			:BRDF(exp, norm)
		{}

		inline glm::vec3 Shade(const glm::vec3 l_vec, const glm::vec3 e_vec,
			const glm::vec3 radiance, const glm::vec3 normal,
			const glm::vec3 material_diffuse, const glm::vec3 material_specular) const
		{
			//Kd * I * cos(theta) /d^2 
			glm::vec3 diffuse = material_diffuse * radiance *
				glm::max(glm::dot(normal, l_vec), 0.0f);
			//Ks* I * max(0, r . n)^s / d^2
			glm::vec3 specular = material_specular * radiance *
				glm::pow(glm::max(0.0f, glm::dot(glm::reflect(-l_vec, normal), glm::normalize(e_vec))), m_exponent);

			return specular + diffuse;
		}
	};
}