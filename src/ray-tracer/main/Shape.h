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
	enum class SHADING_M { flat, smooth };
	class Shape
	{
	public:
		Shape(Material* mat = nullptr, bool visible = true)
			:m_material(mat), m_visible(visible)
		{}
		virtual bool Intersect(const Ray ray, IntersectionData* data) const = 0;
		/*{
			return false;
		}*/

		virtual Bounds3 GetWorldBounds() const = 0;
		/*{
			return Bounds3(glm::vec3(), glm::vec3());
		}*/
		virtual Bounds3 GetLocalBounds() const = 0;

		bool m_visible = true;
		Material* m_material = nullptr;
		glm::mat4* m_transform = nullptr;
		SHAPE_T m_type = SHAPE_T::none;
		glm::vec3 m_motion_blur = { 0,0,0 };
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

					if (norms.size() >= 3)
						m_normals[i] = norms[i];

					if (i != 2 && !uvs.empty())
						m_uvs[i] = uvs[i];
				}
			}
		}

		std::shared_ptr<glm::vec3> m_vertices[3];
		std::shared_ptr<glm::vec3> m_normals[3];
		std::shared_ptr<glm::vec2> m_uvs[3];

		SHADING_M m_shading_mode = SHADING_M::flat;

		Bounds3 GetWorldBounds() const
		{
			glm::vec3 b_min = *m_transform * glm::vec4(*m_vertices[0], 1.0f);
			glm::vec3 b_max = *m_transform * glm::vec4(*m_vertices[0], 1.0f);

			b_min = glm::min(b_min, glm::vec3(*m_transform * glm::vec4(*m_vertices[1], 1.0f)));
			b_min = glm::min(b_min, glm::vec3(*m_transform * glm::vec4(*m_vertices[2], 1.0f)));

			b_max = glm::max(b_max, glm::vec3(*m_transform * glm::vec4(*m_vertices[1], 1.0f)));
			b_max = glm::max(b_max, glm::vec3(*m_transform * glm::vec4(*m_vertices[2], 1.0f)));

			b_min = glm::min(b_min, b_min + m_motion_blur);
			b_max = glm::max(b_max, b_max + m_motion_blur);

			return Bounds3(b_min, b_max);
		}

		Bounds3 GetLocalBounds() const
		{
			glm::vec3 b_min = *m_vertices[0];
			glm::vec3 b_max = *m_vertices[0];

			b_min = glm::min(b_min, *m_vertices[1]);
			b_min = glm::min(b_min, *m_vertices[2]);

			b_max = glm::max(b_max, *m_vertices[1]);
			b_max = glm::max(b_max, *m_vertices[2]);

			return Bounds3(b_min, b_max);
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			Ray inverse_ray;
			glm::mat4 final_transform = *m_transform;
			if (m_motion_blur != glm::vec3(0, 0, 0))
			{
				final_transform = glm::translate(glm::mat4(1.0f), ray.jitter_t * m_motion_blur) * *m_transform;
			}

			inverse_ray.direction = glm::inverse(final_transform) * glm::vec4(ray.direction, 0.0f);
			inverse_ray.origin = glm::inverse(final_transform) * glm::vec4(ray.origin, 1.0f);

			data->t = std::numeric_limits<float>().max();

			/*norms[0] = &m_mesh.m_vertex_normals[m_mesh.m_indices[0]];
			norms[1] = &m_mesh.m_vertex_normals[m_mesh.m_indices[1]];
			norms[2] = &m_mesh.m_vertex_normals[m_mesh.m_indices[2]];*/

			glm::vec3 v0 = *m_vertices[0];
			glm::vec3 v1 = *m_vertices[1];
			glm::vec3 v2 = *m_vertices[2];

			glm::vec3 v0v1 = v1 - v0;
			glm::vec3 v0v2 = v2 - v0;

			glm::vec3 pvec = glm::cross(inverse_ray.direction, v0v2);
			float det = glm::dot(v0v1, pvec);

			data->hit = true;
			if (fabs(det) < inverse_ray.intersect_eps) data->hit = false;

			float invDet = 1 / det;

			glm::vec3 tvec = inverse_ray.origin - v0;
			float u = glm::dot(tvec, (pvec)) * invDet;
			if (u < 0 || u > 1) data->hit = false;

			glm::vec3 qvec = glm::cross(tvec, v0v1);
			float v = glm::dot(inverse_ray.direction, (qvec)) * invDet;
			if (v < 0 || u + v > 1) data->hit = false;

			float t = glm::dot(v0v2, qvec) * invDet;

			if (t < inverse_ray.intersect_eps) data->hit = false;

			/*glm::vec3 t_v0v1, t_v0v2;
			t_v0v1 = glm::transpose(glm::inverse(final_transform)) * glm::vec4(v0v1, 0.0f);
			t_v0v2 = glm::transpose(glm::inverse(final_transform)) * glm::vec4(v0v2, 0.0f);*/
			bool smooth_normals = m_shading_mode == SHADING_M::smooth;
			data->t = t;
			data->position = ray.PointAt(t);
			data->material = m_material;
			data->normal = glm::normalize(glm::mat3(glm::transpose(glm::inverse(final_transform))) *
				( smooth_normals ? 
				(u * (*m_normals[1]) + v * (*m_normals[2]) + (1 - u - v) * (*m_normals[0])) :	//Smooth shading
				(glm::cross(v0v1, v0v2))) );													// Flat shading

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

		Bounds3 GetWorldBounds() const
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

			b_min_f = glm::min(b_min_f, b_min_f + m_motion_blur);
			b_max_f = glm::max(b_max_f, b_max_f + m_motion_blur);


			return Bounds3(b_min_f, b_max_f);
		}

		Bounds3 GetLocalBounds() const
		{
			glm::vec3 t1 = m_center - m_radius * 1.732050f * glm::vec3(1.0f, 1.0f, 1.0f);

			glm::vec3 t2 = m_center + m_radius * 1.732050f * glm::vec3(1.0f, 1.0f, 1.0f);

			glm::vec3 b_min = glm::min(t1, t2); //glm::vec3(1.732050, 1.732050, 1.732050);
			glm::vec3 b_max = glm::max(t1, t2);//glm::vec3(1.732050, 1.732050, 1.732050);

			
			return Bounds3(b_min, b_max);
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			Ray inverse_ray;
			glm::mat4 final_transform = *m_transform;
			if (m_motion_blur != glm::vec3(0, 0, 0))
			{
				final_transform = glm::translate(glm::mat4(1.0f), ray.jitter_t * m_motion_blur) * *m_transform;
			}

			inverse_ray.direction = glm::inverse(final_transform) * glm::vec4(ray.direction, 0.0f);
			inverse_ray.origin = glm::inverse(final_transform) * glm::vec4(ray.origin, 1.0f);

			/*double t;
			glm::vec3 temp = inverse_ray.origin - m_center;
			double a = glm::dot(inverse_ray.direction, inverse_ray.direction);
			double b = 2.0 * glm::dot(temp, inverse_ray.direction);
			double c = glm::dot(temp, temp) - m_radius * m_radius;
			double disc = b * b - 4.0 * a * c;
			if (disc < 0.0)
			{
				data->hit = false;
				return data->hit;
			}
			else
			{
				glm::vec3 transformed_center = final_transform * glm::vec4(m_center, 1.0f);
				data->material = m_material;

				double e = sqrt(disc);
				double denom = 2.0 * a;
				t = (-b - e) / denom; // smaller root
				if (ray.intersect_eps < t) {
					data->hit = true;
					data->t = t;
					data->position = ray.PointAt(t);
					data->normal = glm::normalize(glm::inverse(glm::transpose(final_transform)) *
						glm::vec4((data->position - transformed_center), 0.0f));
					data->uv = glm::vec2(glm::atan(data->position.z, data->position.x),
						glm::acos(data->position.y / m_radius));
					return data->hit;
				}
				t = (-b + e) / denom; // larger root
				if ( ray.intersect_eps < t) {
					data->hit = true;
					data->t = t;
					data->position = ray.PointAt(t);
					data->normal = glm::normalize(glm::inverse(glm::transpose(final_transform)) *
						glm::vec4((data->position - transformed_center), 0.0f));
					data->uv = glm::vec2(glm::atan(data->position.z, data->position.x),
						glm::acos(data->position.y / m_radius));
					return data->hit;
				}

				return false;
			}*/

			float t0, t1;

			float a = glm::dot(inverse_ray.direction, inverse_ray.direction);
			float b = 2.0f * glm::dot(inverse_ray.direction, (inverse_ray.origin - m_center));
			float c = glm::dot(inverse_ray.origin - m_center, inverse_ray.origin - m_center) - m_radius * m_radius;

			double discr = b * b - 4.0 * a * c;
			if (discr < 0.0f)
			{
				data->hit = false;
				return false;
			}
			else {
				float q = (b > 0.0f) ?
					-0.5 * (b + (double)glm::sqrt(discr)) : -0.5 * (b - (double)glm::sqrt(discr));
				t0 = q / a;
				t1 = c / q;
			}

			if (t0 > t1)
				std::swap(t0, t1);

			data->hit = discr >= 0.0f;
			if (t0 < ray.intersect_eps) {
				t0 = t1; // if t0 is negative, let's use t1 instead 
				if (t0 < 0.0) data->hit = false; // both t0 and t1 are negative 
			}

			glm::vec3 transformed_center = final_transform * glm::vec4(m_center, 1.0f);

			data->t = t0;
			data->position = ray.PointAt(t0);
			data->material = m_material;
			data->normal = glm::normalize(glm::inverse(glm::transpose(final_transform)) * glm::vec4((data->position - transformed_center), 0.0f));
			data->uv = glm::vec2(glm::atan(data->position.z, data->position.x),
				glm::acos(data->position.y / m_radius));
			return data->hit;
		}

		float m_radius;
		glm::vec3 m_center = glm::vec3(0, 0, 0);
	};

	class Instance : public Shape
	{
	public:
		Instance(Shape* s, bool reset_transform = false)
			:Shape(*s), m_base_ptr(s), m_reset_transform(reset_transform)
		{
			m_base_ptr = s;
		}

		Bounds3 GetWorldBounds() const
		{

			Bounds3 base_bounds = m_base_ptr->GetLocalBounds();

			base_bounds.min = *m_transform * glm::vec4(base_bounds.min, 1.0f);
			base_bounds.max = *m_transform * glm::vec4(base_bounds.max, 1.0f);

			glm::vec3 b_min, b_max;
			b_min = glm::min(base_bounds.min, base_bounds.max);
			b_max = glm::max(base_bounds.min, base_bounds.max);

			b_min = glm::min(b_min, b_min + m_motion_blur);
			b_max = glm::max(b_max, b_max + m_motion_blur);

			return Bounds3(b_min, b_max);
		}
		
		Bounds3 GetLocalBounds() const
		{
			Bounds3 base_bounds = m_base_ptr->GetLocalBounds();
			/*base_bounds.min = glm::min(base_bounds.min, base_bounds.min + m_motion_blur);
			base_bounds.max = glm::max(base_bounds.max, base_bounds.max + m_motion_blur);*/
			return base_bounds;
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			bool hit = false;
			glm::mat4 final_transform = *m_transform;
			if (m_motion_blur != glm::vec3(0, 0, 0))
			{
				final_transform = glm::translate(glm::mat4(1.0f), ray.jitter_t * m_motion_blur) * *m_transform;
			}
			Ray inv_ray;
			inv_ray.origin = *(m_base_ptr->m_transform) * glm::inverse(final_transform) * glm::vec4(ray.origin, 1.0f);
			inv_ray.direction = *(m_base_ptr->m_transform) * glm::inverse(final_transform) * glm::vec4(ray.direction, 0.0f);
			inv_ray.intersect_eps = ray.intersect_eps;

			if (hit = m_base_ptr->Intersect(inv_ray, data))
			{
				data->normal = glm::normalize(glm::mat3(glm::transpose(glm::inverse(final_transform))) *
					glm::inverse(glm::mat3(glm::transpose(glm::inverse(*(m_base_ptr->m_transform))))) * data->normal);
				data->position = ray.PointAt(data->t);
				if (m_material)
					data->material = m_material;
			}
			return hit;
		}

	private:
		const Shape* m_base_ptr;
		bool m_reset_transform = false;
	};
}