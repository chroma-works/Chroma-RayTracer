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

		glm::vec3 SampleLightDirection(const glm::vec3 isect_pos) const
		{
			//Transform intersenction position to object space
			glm::vec3 obj_isect_pos = *m_inv_transform * glm::vec4(isect_pos, 1.0f);
			float cos_t_max = sqrt(1.0f - pow(1/glm::length(obj_isect_pos), 2.0f));

			float chi_1 = CHR_UTILS::RandFloat(), chi_2 = CHR_UTILS::RandFloat();
			float theta = acosf(1.0f - chi_1 + chi_1 * cos_t_max), phi = 2 * CHR_UTILS::PI * chi_2;

			glm::vec3 w = glm::normalize(glm::vec3(0, 0, 0) - obj_isect_pos), u, v;
			CHR_UTILS::GenerateONB(w, u, v);
			glm::vec3 l_local = w* cos(theta) + v * sin(theta) * cos(phi) + u * sin(theta) * sin(phi);

			Ray ray(isect_pos + l_local * 0.00001f, glm::normalize(*m_transform * glm::vec4(l_local, 0.0f)));//p_world, l_world
			IntersectionData light_isect;
			Sphere::Intersect(ray, &light_isect);

			return glm::normalize(light_isect.position - isect_pos);
		}

		glm::vec3 RadianceAt(const glm::vec3 isect_pos, const glm::vec3 l_vec) const
		{
			//Transform intersenction position to object space
			glm::vec3 obj_isect_pos = *m_inv_transform * glm::vec4(isect_pos, 1.0f);
			float cos_t_max = sqrt(1 - pow(1 / glm::length(obj_isect_pos), 2.0f));

			Ray ray(isect_pos + l_vec * 0.00001f, l_vec);//p_world, l_world
			IntersectionData isect_data;
			Sphere::Intersect(ray, &isect_data);
			glm::vec3 pos = isect_data.position;

			float d = glm::distance(pos, isect_pos);
			return m_inten / (d * d) * ((float)CHR_UTILS::PI * 2.0f * (1.0f - cos_t_max));
		}
		
		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			bool hit = Sphere::Intersect(ray, data);
			if(hit)
				data->radiance = m_inten;
			return hit;
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
		}

		LightTriangle(glm::vec3 radiance,
			std::shared_ptr<Triangle> tri)
			: Triangle(*tri)
		{
			m_inten = radiance;
			m_li_type = LIGHT_T::object;
			m_area = CHR_UTILS::CalculateTriangleArea(*m_vertices[0], *m_vertices[1], *m_vertices[2]);
		}

		glm::vec3 SampleLightDirection(const glm::vec3 isect_pos) const
		{
			float chi_1 = CHR_UTILS::RandFloat(), chi_2 = CHR_UTILS::RandFloat();
			glm::vec3 p = (1.0f - chi_2) * *m_vertices[1] + chi_2 * *m_vertices[2];
			glm::vec3 q = sqrt(chi_1) * p + (1.0f - sqrt(chi_1)) * *m_vertices[0];

			return glm::normalize(q - isect_pos);
		}

		glm::vec3 RadianceAt(const glm::vec3 isect_pos, const glm::vec3 l_vec) const
		{
			Ray ray(isect_pos + l_vec * 0.00001f, l_vec);
			IntersectionData data;
			Triangle::Intersect(ray, &data);
			glm::vec3 normal;
			for (int i = 0; i < 3; i++)
			{
				normal += glm::distance(*m_vertices[i], data.position)* *m_normals[i];
			}
			normal = glm::normalize(normal);
			float cos_t = abs(glm::dot(l_vec, normal));

			float d = glm::distance(isect_pos, data.position);
			return m_inten * m_area * cos_t / (d * d);
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			bool hit = Triangle::Intersect(ray, data);
			if (hit)
				data->radiance = m_inten;
			return hit;
		}

		float GetArea()
		{
			return m_area > 0.0f ? m_area : CHR_UTILS::CalculateTriangleArea(*m_vertices[0], *m_vertices[1], *m_vertices[2]);
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
			m_area_totals.resize(m_triangles.size());
			float running_total = 0.0f;
			int i = 0;
			for (auto tri : m_triangles)
			{
				running_total += tri->GetArea();
				m_area_totals[i++] = running_total;
			}
			m_surface_area = m_area_totals[--i];
		}

		glm::vec3 SampleLightDirection(const glm::vec3 isect_pos) const
		{
			//Select random triangle based on their surface area
			float r = CHR_UTILS::RandFloat(0.0f, m_surface_area);
			int i;
			for (i = 0; i < m_triangles.size(); i++)
				if (r < m_area_totals[i])
					break;
			
			return m_triangles[i]->SampleLightDirection(isect_pos);
		}

		glm::vec3 RadianceAt(const glm::vec3 isect_pos, const glm::vec3 l_vec) const
		{
			//Find the triangle(or one that is on the same path)
			IntersectionData probe_data;
			Ray ray(isect_pos + l_vec * 0.00001f, l_vec);
			int i;
			for (i = 0; i < m_triangles.size(); i++)
				if (m_triangles[i]->Intersect(ray, &probe_data))
					break;
			int j = glm::min(i, (int)m_triangles.size() - 1);	//TODO:FIX!!!!!
			if (probe_data.hit)
				return m_triangles[j]->RadianceAt(isect_pos, l_vec) * m_surface_area / m_triangles[j]->GetArea();
			else
				return { 0, 0, 0 };
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

			if(data->hit)
				data->radiance = m_inten;

			return data->hit;
		}

		Bounds3 GetWorldBounds() const
		{
			return m_triangles[0]->GetWorldBounds();	//OBVIOUSLY WRONG
		}
		Bounds3 GetLocalBounds() const
		{
			return m_triangles[0]->GetLocalBounds();	//OBVIOUSLY WRONG
		}
		glm::vec3 ObjectSpaceNormalAt(glm::vec3 p, glm::vec3 normal, glm::vec2 uv) const
		{
			return m_triangles[0]->ObjectSpaceNormalAt(p, normal, uv);	//OBVIOUSLY WRONG
		}

		void DrawGUI()
		{}

	private:
		std::vector<float> m_area_totals; //for random triangle selection
	};
}
