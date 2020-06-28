#pragma once
#include <ray-tracer/main/Utilities.h>

namespace CHR
{

	enum class BRDF_T { bl_ph, mod_bl_ph, ph, mod_ph, tor_spa };
	class BRDF
	{
	public:
		BRDF(float exp = 1.0f, bool norm = false)
			:m_exponent(exp), m_normalized(norm)
		{}
		float m_exponent;
		bool m_normalized;
		BRDF_T m_type;

	protected:
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
		{
			m_type = BRDF_T::bl_ph;
		}
	protected:
		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{ 
			return glm::dot(normal, l_vec) > 0.0f ?
				1.0f : 0.0f;
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			//max(0, h . n)^s / cos(theta)
			float cos_t;
			if (cos_t = glm::dot(normal, l_vec) > 0.0f)
			{
				glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
				return	glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), m_exponent) / cos_t;
			}
			return 0.0f;
		}
	};

	class Phong : public BRDF
	{
	public:
		Phong(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{
			m_type = BRDF_T::ph;
		}
	protected:
		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			return glm::dot(normal, l_vec) > 0.0f ?
				1.0f : 0.0f;
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			//max(0, r . n)^s
			float cos_t;
			if (cos_t = glm::dot(normal, l_vec) > 0.0f)
			{
				glm::vec3 r = glm::normalize(glm::reflect(-l_vec, normal));
				return	glm::pow(glm::max(0.0f, glm::dot(r, glm::normalize(e_vec))), m_exponent) / cos_t;
			}
			else
				return 0.0f;
		}
	};

	class ModifiedBlinnPhong : public BRDF
	{
	public:
		ModifiedBlinnPhong(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{
			m_type = BRDF_T::mod_bl_ph;
		}
	protected:
		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			if (glm::dot(normal, l_vec) > 0.0f)
			{
				if (m_normalized)// 1.0f / pi
					return 1.0f / CHR_UTILS::PI;
				else
					return 1.0f;
			}
			return 0.0f;
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			float cos_t;
			if (cos_t = glm::dot(normal, l_vec) > 0.0f)
			{
				glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
				if (m_normalized)
					return glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), m_exponent) *
					0.125f * (m_exponent + 8.0f) / CHR_UTILS::PI / cos_t;
				else
					return glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(normal))), m_exponent) / cos_t;
			}
			return 0.0f;
		}
	};

	class ModifiedPhong : public BRDF
	{
	public:
		ModifiedPhong(float exp = 1.0f, bool norm = false)
			:BRDF(exp, norm)
		{
			m_type = BRDF_T::mod_ph;
		}
	protected:
		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			if (glm::dot(normal, l_vec) > 0.0f)
			{
				if (m_normalized)// 1.0f/pi
					return 1.0f / CHR_UTILS::PI;
				else
					return 1.0f;
			}
			return 0.0f;
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			float cos_t;
			if (cos_t = glm::dot(normal, l_vec) > 0.0f)
			{
				glm::vec3 r = glm::normalize(glm::reflect(-l_vec, normal));
				if (m_normalized)
					return glm::pow(glm::max(0.0f, glm::dot(r, glm::normalize(e_vec))), m_exponent) *
					0.5f * (m_exponent + 2.0f) / CHR_UTILS::PI / cos_t;
				else
					return glm::pow(glm::max(0.0f, glm::dot(r, glm::normalize(e_vec))), m_exponent) *
					glm::max(glm::dot(normal, l_vec), 0.0f) / cos_t;
			}
			return 0.0f;
		}
	};

	class TorranceSparrow : public BRDF
	{
	public:
		TorranceSparrow(bool kd_fresnel, float exp = 1.0f, bool norm = false)
			:kd_fresnel(kd_fresnel), BRDF(exp, norm)
		{
			m_type = BRDF_T::tor_spa;
		}
		void SetFresnelCoeffs(float ts_ref_ind, float ts_abs_ind)
		{
			m_refraction_ind = ts_ref_ind;
			m_absorption_ind = ts_abs_ind;
		}
	protected:
		float CalculateDiffuse(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			if (glm::dot(normal, l_vec) > 0.0f)
			{
				if (kd_fresnel)
				{
					glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
					return (1.0f - CalculateFresnell(glm::dot(h, e_vec))) / CHR_UTILS::PI;
				}
				else
					return 1.0f / CHR_UTILS::PI;
			}
			return 0.0f;
		}

		float CalculateSpecular(const glm::vec3 l_vec,
			const glm::vec3 e_vec, const glm::vec3 normal) const
		{
			float cos_t;
			if (cos_t = glm::dot(normal, l_vec) > 0.0f)
			{
				glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));

				float g_term = glm::min(1.0f,
					glm::min(2.0f * (glm::dot(normal, h) * glm::dot(normal, e_vec) / (glm::dot(e_vec, h))),
						2.0f * (glm::dot(normal, h) * glm::dot(l_vec, normal) / (glm::dot(l_vec, h)))));
				float d_term = (m_exponent + 2.0f) / (CHR_UTILS::PI) *
					glm::max(glm::pow(glm::dot(h, normal), m_exponent), 0.0f);
				float f_term = CalculateFresnell(glm::dot(h, e_vec));
				return g_term * d_term * f_term / (4.0f * glm::dot(e_vec, normal) * cos_t);
			}
			return 0.0f;
		}

		float CalculateFresnell(float cos_i) const
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
		float m_refraction_ind;
		float m_absorption_ind;
		bool kd_fresnel = false;
	};

}