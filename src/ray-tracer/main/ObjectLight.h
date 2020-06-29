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
			m_li_type = LIGHT_T::object;
		}
		
		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			bool hit = Sphere::Intersect(ray, data);
			if (hit)
				data->radiance = m_inten; //RadianceAt(data->position, ray.direction);
			return hit;
		}

		glm::vec3 SampleRadianceAt(const glm::vec3 isect_pos, glm::vec3& l_vec) const
		{
			//Transform intersenction position to object space
			glm::vec3 obj_isect_pos = *m_inv_transform * glm::vec4(isect_pos, 1.0f);
			float cos_t_max = sqrt(1.0f - pow(1 / glm::length(obj_isect_pos), 2.0f));

			float chi_1 = CHR_UTILS::RandFloat(), chi_2 = CHR_UTILS::RandFloat();
			float theta = acosf(1.0f - chi_1 + chi_1 * cos_t_max), phi = 2 * CHR_UTILS::PI * chi_2;

			glm::vec3 w = glm::normalize(glm::vec3(0, 0, 0) - obj_isect_pos), u, v;
			CHR_UTILS::GenerateONB(w, u, v);
			glm::vec3 l_local = w * cos(theta) + v * sin(theta) * cos(phi) + u * sin(theta) * sin(phi);

			Ray ray(isect_pos + l_local * 0.00001f, glm::normalize(*m_transform * glm::vec4(l_local, 0.0f)));//p_world, l_world
			IntersectionData light_isect;
			Sphere::Intersect(ray, &light_isect);

			l_vec = glm::normalize(light_isect.position - isect_pos);
			float d = glm::distance(light_isect.position, isect_pos);
			return m_inten / (d * d) * ((float)CHR_UTILS::PI * 2.0f * (1.0f - cos_t_max));
		}

		void DrawGUI()
		{}

	};

	class LightTriangle : public Light, public Triangle
	{
	public:

		LightTriangle(glm::vec3 radiance, 
			std::vector<std::shared_ptr<glm::vec3>>verts,
			std::vector<std::shared_ptr<glm::vec3>> norms,
			std::vector<std::shared_ptr<glm::vec2>> uvs, 
			std::shared_ptr<Material> mat, bool visible = true)
			: Triangle(verts, norms, uvs, mat, visible)
		{
			m_inten = radiance;
			m_li_type = LIGHT_T::object;
			m_area = CHR_UTILS::CalculateTriangleArea(*m_vertices[0], *m_vertices[1], *m_vertices[2]);
			m_shading_mode = SHADING_MODE::smooth;
		}

		LightTriangle(glm::vec3 radiance,
			std::shared_ptr<Triangle> tri)
			: Triangle(*tri)
		{
			m_inten = radiance;
			m_li_type = LIGHT_T::object;
			m_area = CHR_UTILS::CalculateTriangleArea(*m_vertices[0], *m_vertices[1], *m_vertices[2]);
			m_shading_mode = SHADING_MODE::smooth;
		}

		float GetArea()
		{
			return m_area > 0.0f ? m_area : CHR_UTILS::CalculateTriangleArea(*m_vertices[0], *m_vertices[1], *m_vertices[2]);
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			bool hit = Triangle::Intersect(ray, data);
			if (hit)
				data->radiance = m_inten; //RadianceAt(data->position, ray.direction);
			return hit;
		}

		glm::vec3 SampleRadianceAt(const glm::vec3 isect_pos, glm::vec3& l_vec) const
		{
			float chi_1 = CHR_UTILS::RandFloat(), chi_2 = CHR_UTILS::RandFloat();
			glm::vec3 p = (1.0f - chi_2) * glm::vec3(*m_transform * glm::vec4(*m_vertices[1], 1.0f)) +
				chi_2 * glm::vec3(*m_transform * glm::vec4(*m_vertices[2], 1.0f));
			glm::vec3 q = sqrt(chi_1) * p +
				(1.0f - sqrt(chi_1)) * glm::vec3(*m_transform * glm::vec4(*m_vertices[0], 1.0f));

			l_vec = glm::normalize(q - isect_pos);

			Ray ray(isect_pos, l_vec);
			ray.intersect_eps = 0.0000001f;
			IntersectionData data;
			Triangle::Intersect(ray, &data);
			glm::vec3 normal;
			if (data.hit)
			{

				float cos_t = abs(glm::dot(-l_vec, data.normal));
				/*if (isnan(cos_t))
					cos_t = abs(glm::dot(-l_vec, glm::normalize(*m_normals[2] + *m_normals[1] + *m_normals[0])));*/

				float d = glm::distance(ray.origin, data.position);
				return m_inten * cos_t / (d * d) * m_area;	//TODO:Fix
			}
			else
				return { 0,0,0 };
		}

		void DrawGUI()
		{}
	private:
		float m_area = -1.0f;
	};

	class LightMesh : public Light, public Shape
	{
	public:
		std::vector<std::shared_ptr<LightTriangle>> m_triangles;
		float m_surface_area = 0.0;

		LightMesh(glm::vec3 radiance, std::vector<std::shared_ptr<LightTriangle>> tris)
			: Shape(tris[0]->m_material, tris[0]->m_visible), m_triangles(tris)
		{
			m_inten = radiance;
			m_li_type = LIGHT_T::object;
			m_cumulative_areas;
			float running_total = 0.0f;
			int i = 0;
			for (auto tri : m_triangles)
			{
				running_total += tri->GetArea();
				m_cumulative_areas[running_total] = i++;
			}
			m_surface_area = running_total;
		}


		bool Intersect(const Ray ray, IntersectionData* data) const //BRUTE FORCE! CALLING FREQUENTLY WILL CAUSE MAJOR PERF. LOSS
		{
			data->t = INFINITY;

			IntersectionData probe_data;
			for (auto tri : m_triangles)
			{
				tri->Intersect(ray, &probe_data);
				if (probe_data.t < data->t)
					*data = probe_data;
			}

			//if (data->hit)
				data->radiance = m_inten; //* glm::max(0.0f, -glm::dot(data->normal, ray.direction)); //RadianceAt(data->position, ray.direction);

			return data->hit;
		}

		Bounds3 GetWorldBounds() const
		{
			auto b_min = m_triangles[0]->GetWorldBounds().min;
			auto b_max = m_triangles[0]->GetWorldBounds().max;
			for (auto tri : m_triangles)
			{
				b_min = glm::min(b_min, tri->GetWorldBounds().min);
				b_max = glm::max(b_max, tri->GetWorldBounds().max);
			}
			return Bounds3(b_min, b_max);
		}
		Bounds3 GetLocalBounds() const
		{
			auto b_min = m_triangles[0]->GetLocalBounds().min;
			auto b_max = m_triangles[0]->GetLocalBounds().max;
			for (auto tri : m_triangles)
			{
				b_min = glm::min(b_min, tri->GetLocalBounds().min);
				b_max = glm::max(b_max, tri->GetLocalBounds().max);
			}
			return Bounds3(b_min, b_max);
		}
		glm::vec3 ObjectSpaceNormalAt(glm::vec3 p, glm::vec3 normal, glm::vec2 uv) const
		{
			return m_triangles[0]->ObjectSpaceNormalAt(p, normal, uv);	//OBVIOUSLY WRONG
		}

		glm::vec3 SampleRadianceAt(const glm::vec3 isect_pos, glm::vec3& l_vec) const
		{
			float r = CHR_UTILS::RandFloat() * (m_surface_area - 0.00001f);
			auto rad = m_triangles[m_cumulative_areas.upper_bound(r)->second]->SampleRadianceAt(isect_pos, l_vec);
			return rad * m_surface_area / m_triangles[m_cumulative_areas.upper_bound(r)->second]->GetArea();
		}

		void DrawGUI()
		{}

	private:
		std::map<float, int> m_cumulative_areas; //for random triangle selection
	};
}
