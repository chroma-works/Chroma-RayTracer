#pragma once
#include <ray-tracer/main/Utilities.h>

namespace CHR
{

	//class CHR::Material;
	class BRDF
	{
	public:
		BRDF(float exp = 1.0f, bool norm = false)
			:m_exponent(exp), m_normalized(norm)
		{}
		float m_exponent;
		bool m_normalized;

	private:
		friend class Material;

		virtual float CalculateDiffuse(const glm::vec3 l_vec, 
			const glm::vec3 e_vec, const glm::vec3 normal) const = 0;

		virtual float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const = 0;
	};

	class BlinnPhong : public BRDF
	{
	public:
		BlinnPhong(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{}

		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			//cos(theta)  
			return glm::max(glm::dot(normal, l_vec), 0.0f);
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			//max(0, h . n)^s 
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			return	glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), m_exponent);
		}
	};

	class Phong : public BRDF
	{
	public:
		Phong(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{}

		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			//cos(theta)  
			return glm::max(glm::dot(normal, l_vec), 0.0f);
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			//max(0, r . n)^s 
			glm::vec3 r = glm::normalize(glm::reflect(-l_vec, normal));
			return	glm::pow(glm::max(0.0f, glm::dot(r, glm::normalize(e_vec))), m_exponent);
		}
	};

	class ModifiedBlinnPhong : public BRDF
	{
	public:
		ModifiedBlinnPhong(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{}

		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			if (m_normalized)// cos(theta) / pi
				return glm::max(glm::dot(normal, l_vec), 0.0f) * 1.0f / CHR_UTILS::PI;
			else//cos(theta)
				return glm::max(glm::dot(normal, l_vec), 0.0f);
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			if (m_normalized)
				return glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), m_exponent) *
					glm::max(glm::dot(normal, l_vec), 0.0f) * 0.125f * (m_exponent + 8.0f) / CHR_UTILS::PI;
			else
				return glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), m_exponent) *
					glm::max(glm::dot(normal, l_vec), 0.0f);
		}
	};

	class ModifiedPhong : public BRDF
	{
	public:
		ModifiedPhong(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{}

		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			if (m_normalized)// cos(theta) / pi
				return glm::max(glm::dot(normal, l_vec), 0.0f) * 1.0f / CHR_UTILS::PI;
			else//cos(theta)
				return glm::max(glm::dot(normal, l_vec), 0.0f);
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			glm::vec3 r = glm::normalize(glm::reflect(-l_vec, normal));
			if (m_normalized)
				return glm::pow(glm::max(0.0f, glm::dot(r, glm::normalize(e_vec))), m_exponent) *
				glm::max(glm::dot(normal, l_vec), 0.0f) * 0.5f * (m_exponent + 2.0f) / CHR_UTILS::PI;
			else
				return glm::pow(glm::max(0.0f, glm::dot(r, glm::normalize(e_vec))), m_exponent) *
				glm::max(glm::dot(normal, l_vec), 0.0f);
		}
	};

	/*class TorranceSparrow : public BRDF
	{
	public:
		TorranceSparrow(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{}

		inline glm::vec3 Shade(const glm::vec3 l_vec, const glm::vec3 e_vec,
			const glm::vec3 radiance, const glm::vec3 normal,
			const glm::vec3 kd, const glm::vec3 ks) const
		{
			//Kd * cos(theta) / pi
			glm::vec3 diffuse = kd * radiance *
				glm::max(glm::dot(normal, l_vec), 0.0f);
			diffuse *= 1.0f / CHR_UTILS::PI;

			//specular
			glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
			glm::vec3 w0 = glm::reflect(-l_vec, h);
			glm::vec3 specular = ks * radiance;
			float g_term = glm::min(1.0f,
				glm::min(2.0f * (glm::dot(normal, h) * glm::dot(normal, w0) / (glm::dot(w0, h))), 
					2.0f * (glm::dot(normal, h) * glm::dot(-l_vec, normal)/(glm::dot(l_vec, h))) ));

			return specular + diffuse;
		}
	};*/
}