#pragma once


#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/gtx/intersect.hpp>
#include <thirdparty/glm/glm/gtc/constants.hpp>
#include <ray-tracer/main/Geometry.h>
#include <ray-tracer/main/Material.h>
#include <ray-tracer/main/Ray.h>

namespace Chroma
{
	enum class SHAPE_T { none, triangle, sphere };
	class Shape
	{
	public:
		Shape(Material* mat= nullptr, bool visible=true)
			:m_material(mat), m_is_visible(visible)
		{}
		virtual bool Intersect(const Ray ray, IntersectionData* data) const = 0;
		/*{
			return false;
		}*/

		virtual Bounds3 GetBounds() const = 0;
		/*{
			return Bounds3(glm::vec3(), glm::vec3());
		}*/

		bool m_is_visible = true;
		Material* m_material = nullptr;
		glm::mat4* m_transform = nullptr;
		SHAPE_T m_type = SHAPE_T::none;
	};
	class Triangle : public Shape
	{
	public:
		Triangle(Material* mat, bool visible = true)
			:Shape(mat, visible)
		{
			m_type = SHAPE_T::triangle;
		}
		Triangle(std::vector<std::shared_ptr<glm::vec3>>verts, 
			std::vector<std::shared_ptr<glm::vec3>> norms,
			std::vector<std::shared_ptr<glm::vec2>> uvs, 
			Material* mat, bool visible = true)
			: Shape(mat, visible)
		{
			m_type = SHAPE_T::triangle;
			if (verts.size() != 3)
				CH_WARN("Non-Triangle object passed to Triangle class");
			if (verts.size() >= 3)
			{
				for (size_t i = 0; i < 3; i++)
				{
					m_vertices[i] = verts[i];

					if(norms.size() >=3)
						m_normals[i] = norms[i];

					if (i != 2 && !uvs.empty())
						m_uvs[i] = uvs[i];
				}
			}
		}
		std::shared_ptr<glm::vec3> m_vertices[3];
		std::shared_ptr<glm::vec3> m_normals[3];
		std::shared_ptr<glm::vec2> m_uvs[3];

		Bounds3 GetBounds() const
		{
			glm::vec3 b_min = *m_transform * glm::vec4(*m_vertices[0],1.0f);
			glm::vec3 b_max = *m_transform * glm::vec4(*m_vertices[0],1.0f);

			b_min = glm::min(b_min, glm::vec3(*m_transform * glm::vec4(*m_vertices[1], 1.0f)));
			b_min = glm::min(b_min, glm::vec3(*m_transform * glm::vec4(*m_vertices[2], 1.0f)));

			b_max = glm::max(b_max, glm::vec3(*m_transform * glm::vec4(*m_vertices[1], 1.0f)));
			b_max = glm::max(b_max, glm::vec3(*m_transform * glm::vec4(*m_vertices[2], 1.0f)));

			return Bounds3(b_min, b_max);
		}

		bool Intersect(const Ray _ray, IntersectionData* data) const
		{
			Ray ray;
			ray.direction = glm::inverse(*m_transform) * glm::vec4(_ray.direction, 1.0f);
			ray.origin = glm::inverse(*m_transform) * glm::vec4(_ray.origin, 1.0f);

			data->t = std::numeric_limits<float>().max();

			/*norms[0] = &m_mesh.m_vertex_normals[m_mesh.m_indices[0]];
			norms[1] = &m_mesh.m_vertex_normals[m_mesh.m_indices[1]];
			norms[2] = &m_mesh.m_vertex_normals[m_mesh.m_indices[2]];*/

			glm::vec3 v0 = *m_vertices[0];
			glm::vec3 v1 = *m_vertices[1];
			glm::vec3 v2 = *m_vertices[2];

			glm::vec3 v0v1 = v1 - v0;
			glm::vec3 v0v2 = v2 - v0;

			glm::vec3 pvec = glm::cross(ray.direction, v0v2);
			float det = glm::dot(v0v1, pvec);

			data->hit = true;
			if (fabs(det) < ray.intersect_eps) data->hit = false;

			float invDet = 1 / det;

			glm::vec3 tvec = ray.origin - v0;
			float u = glm::dot(tvec, (pvec)) * invDet;
			if (u < 0 || u > 1) data->hit = false;

			glm::vec3 qvec = glm::cross(tvec, v0v1);
			float v = glm::dot(ray.direction, (qvec)) * invDet;
			if (v < 0 || u + v > 1) data->hit = false;

			float t = glm::dot(v0v2, qvec) * invDet;

			if (t < ray.intersect_eps) data->hit = false;

			data->t = t;
			data->position = ray.PointAt(t);
			data->material = m_material;
			data->normal = glm::transpose(glm::inverse(glm::mat3(*m_transform))) *
				glm::normalize(glm::cross(v0v1, v0v2)); //u *(*normals[1]) + v * (*normals[2]) + (1 - u - v) * (*normals[0]); //Smooth shading

			return data->hit;
		}
	};

	class Sphere : public Shape
	{
	public:
		Sphere(Material* mat, bool visible = true)
			:Shape(mat, visible)
		{
			m_radius = 0.0f;
			m_type = SHAPE_T::sphere;
		}

		Sphere(float rad, Material* mat, bool visible = true)
			:Shape(mat, visible), m_radius(rad)
		{
			m_type = SHAPE_T::sphere;
		}

		Bounds3 GetBounds() const
		{
			glm::vec3 t1 = m_center - m_radius * 1.732050f * glm::vec3(1.0f, 1.0f, 1.0f);
			//t1 = glm::vec3(*m_transform * glm::vec4(t1, 1.0f));

			glm::vec3 t2 = m_center + m_radius * 1.732050f * glm::vec3(1.0f, 1.0f, 1.0f);
			//t2 = glm::vec3(*m_transform * glm::vec4(t2, 1.0f));

			glm::vec3 b_min = glm::min(t1, t2); //glm::vec3(1.732050, 1.732050, 1.732050);
			glm::vec3 b_max = glm::max(t1, t2);//glm::vec3(1.732050, 1.732050, 1.732050);

			b_min = *m_transform * glm::vec4(b_min, 1.0f);
			b_max = *m_transform * glm::vec4(b_max, 1.0f);

			glm::vec3 b_min_f = glm::min(b_min, b_max);
			glm::vec3 b_max_f = glm::max(b_min, b_max);

			return Bounds3(b_min_f, b_max_f);
		}

		bool Intersect(const Ray _ray, IntersectionData* data) const
		{
			Ray ray;
			ray.direction = glm::inverse(*m_transform) * glm::vec4(_ray.direction, 1.0f);
			ray.origin = glm::inverse(*m_transform) * glm::vec4(_ray.origin, 1.0f);

			float a = glm::dot(ray.direction, ray.direction);
			float b = 2.0f * glm::dot(ray.direction, (ray.origin - m_center));
			float c = glm::dot(ray.origin - m_center, ray.origin - m_center) - m_radius * m_radius;

			float t0 = std::numeric_limits<float>().max(), t1 = std::numeric_limits<float>().max();
			double discr = b * b - 4 * a * c;
			if (discr < ray.intersect_eps)
			{
				data->hit = false;
				return false;
			}
			else if (discr == ray.intersect_eps) //single root
				t0 = t1 = -0.5 * b / a;
			else {
				float q = (b > 0.0f) ?
					-0.5 * (b + (double)glm::sqrt(discr)) : -0.5 * (b - (double)glm::sqrt(discr));
				t0 = q / a;
				t1 = c / q;
			}
			if (t0 > t1)
				std::swap(t0, t1);

			data->hit = discr >= ray.intersect_eps;
			if (t0 < 0.0f) {
				t0 = t1; // if t0 is negative, let's use t1 instead 
				if (t0 < 0.7f) data->hit=false; // both t0 and t1 are negative 
			}
			data->t = t0;
			data->material = m_material;
			data->position = ray.PointAt(t0);
			data->normal = glm::normalize(glm::transpose(glm::inverse(glm::mat3(*m_transform))) * (data->position - m_center));
			data->uv = glm::vec2(glm::atan(data->position.z, data->position.x),
				glm::acos(data->position.y / m_radius));
			return data->hit;
		}

		float m_radius;
		glm::vec3 m_center = glm::vec3(0,0,0);
	};
}