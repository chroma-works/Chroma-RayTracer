#pragma once

#include "Light.h"
#include "Shape.h"

namespace CHR
{
	class LightSphere : public Light, public Sphere
	{
	public:
		LightSphere(glm::vec3 radiance, std::shared_ptr<Material> mat, bool visible = true)
			:Sphere(mat, visible)
		{
			m_inten = radiance;
			m_li_type = LIGHT_T::mesh;
		}

		glm::vec3 SampleLightDirection(const glm::vec3 isect_pos) const
		{
			//Transform intersenction position to object space
			glm::vec3 obj_isect_pos = *m_inv_transform * glm::vec4(isect_pos, 1.0f);
			float cos_t_max = sqrt(1 - pow(1/glm::length(obj_isect_pos), 2.0f));

			float chi_1 = CHR_UTILS::RandFloat(), chi_2 = CHR_UTILS::RandFloat();
			float theta = acosf(1.0f - chi_1 + chi_1 * cos_t_max), phi = 2 * CHR_UTILS::PI * chi_2;

			glm::vec3 w = glm::normalize(glm::vec3(0, 0, 0) - obj_isect_pos), u, v;
			CHR_UTILS::GenerateONB(w, u, v);
			glm::vec3 l_local = w* cos(theta) + v * sin(theta) * cos(phi) + u * sin(theta) * cos(phi);

			/*Ray ray(isect_pos, glm::normalize(*m_transform * glm::vec4(l_local, 0.0f)));//p_world, l_world
			IntersectionData isect_data;
			
			Intersect(ray, &isect_data);*/
			return glm::normalize(*m_inv_transform * glm::vec4(l_local,0));
		}

		glm::vec3 RadianceAt(const glm::vec3 isect_pos, const glm::vec3 l_vec) const
		{
			//Transform intersenction position to object space
			glm::vec3 obj_isect_pos = *m_inv_transform * glm::vec4(isect_pos, 1.0f);
			float cos_t_max = sqrt(1 - pow(1 / glm::length(obj_isect_pos), 2.0f));

			Ray ray(isect_pos, l_vec);//p_world, l_world
			IntersectionData isect_data;
			Intersect(ray, &isect_data);
			glm::vec3 pos = isect_data.position;

			float d = glm::distance(pos, isect_pos);
			return m_inten / (d * d) / ((float)CHR_UTILS::PI * 2.0f * (1.0f - cos_t_max));
		}

		void DrawGUI()
		{}

	};

	/*class LightTriangle : public Light, public Triangle
	{

	};*/
}
