#pragma once

#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/gtx/intersect.hpp>
#include <thirdparty/glm/glm/gtc/constants.hpp>
#include <thirdparty/glm/glm/gtx/vector_angle.hpp>
#include <thirdparty/glm/glm/gtc/matrix_access.hpp>
#include <ray-tracer/main/Geometry.h>
#include <ray-tracer/main/Material.h>
#include <ray-tracer/main/Ray.h>

namespace CHR
{
	enum class SHAPE_T { none, triangle, sphere };
	enum class SHADING_MODE { flat, smooth };

	class Shape
	{
	public:
		Shape(std::shared_ptr<Material> mat = nullptr, bool visible = true)
			:m_material(mat), m_visible(visible)
		{}
		virtual bool Intersect(const Ray ray, IntersectionData* data) const = 0;
		inline void SetTransform(glm::mat4* transform, glm::mat4* inv_transform)
		{
			m_transform = transform;
			m_inv_transform = inv_transform;
		}

		virtual Bounds3 GetWorldBounds() const = 0;
		virtual Bounds3 GetLocalBounds() const = 0;
		virtual glm::vec3 ObjectSpaceNormalAt(glm::vec3 p, glm::vec3 normal, glm::vec2 uv) const = 0;

		bool m_visible = true;
		std::shared_ptr<Material> m_material = nullptr;
		std::shared_ptr<TextureMap> m_tex_maps[2] = {nullptr, nullptr};		//0 = shading, 1 = normal perturbation
		SHAPE_T m_shape_type = SHAPE_T::none;
		glm::vec3 m_motion_blur = { 0,0,0 };

	protected:
		friend class Instance;
		glm::mat4* m_transform = nullptr;
		glm::mat4* m_inv_transform = nullptr;
	};
	class Triangle : public Shape
	{
	public:
		Triangle(std::shared_ptr<Material> mat, bool visible = true)
			:Shape(mat, visible)
		{
			m_shape_type = SHAPE_T::triangle;
		}
		Triangle(std::vector<std::shared_ptr<glm::vec3>>verts,
			std::vector<std::shared_ptr<glm::vec3>> norms,
			std::vector<std::shared_ptr<glm::vec2>> uvs,
			std::shared_ptr<Material> mat, bool visible = true)
			: Shape(mat, visible)
		{
			m_shape_type = SHAPE_T::triangle;
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

		SHADING_MODE m_shading_mode = SHADING_MODE::flat;

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

		glm::vec3 ObjectSpaceNormalAt(glm::vec3 p, glm::vec3 normal, glm::vec2 uv) const
		{
			glm::vec3 tangent_normal = 
				glm::normalize(
					m_tex_maps[1]->SampleAt(glm::vec3(uv.x * (*m_uvs[1]) + uv.y * (*m_uvs[2]) + (1 - uv.x -uv.y) * (*m_uvs[0]), NAN))
				);
			glm::mat2 A_inv = glm::inverse(glm::mat2(
				*m_uvs[2] - *m_uvs[1],
				*m_uvs[0] - *m_uvs[1]));

			glm::mat2x3 E = { {*m_vertices[2] - *m_vertices[1] }, {*m_vertices[0] - *m_vertices[1]} };

			glm::mat2x3 TB = E * A_inv;
			glm::vec3 N = glm::cross(*m_vertices[2] - *m_vertices[1], *m_vertices[0] - *m_vertices[1]);
			glm::mat3 TBN = {glm::column(TB,0), glm::column(TB,1), N};

			if (m_tex_maps[1]->GetDecalMode() == DECAL_M::bump)
			{
				if (m_tex_maps[1]->GetType() == SOURCE_T::noise)
				{
					glm::vec3 g = m_tex_maps[1]->BumpAt(*m_inv_transform * glm::vec4(p, 1));
					return normal - (g - (glm::dot(g, normal) * normal));
				}
				else //Image
				{
					glm::vec2 dudv = m_tex_maps[1]->BumpAt(glm::vec3(uv.x * (*m_uvs[1]) + uv.y * (*m_uvs[2]) + (1 - uv.x - uv.y) * (*m_uvs[0]), NAN));
					auto normal_prime = glm::normalize(glm::normalize(normal) - dudv.x * glm::normalize(TB[1]) - dudv.y * glm::normalize(TB[0]));
					if (glm::all(glm::isnan(TBN[0])) || glm::all(glm::isnan(TBN[1])))
					{
						return glm::normalize(normal);
					}
					return abs(glm::dot(normal, normal_prime)) < 10e-6 ? -normal_prime : normal_prime;
					//glm::vec3 dpPrimedu = glm::normalize(TBN[1]) + dudv.x * glm::normalize(normal);
					//glm::vec3 dpPrimedv = glm::normalize(TBN[0]) + dudv.y * glm::normalize(normal);
					//glm::vec3 normal_prime = glm::normalize(glm::cross(dpPrimedv, dpPrimedu));
					//
					//float angle = glm::angle(normal, normal_prime);
					//return /*abs(angle) > glm::pi<float>()/2 ? -normal_prime :*/ normal_prime;
				}
			}

			else if(m_tex_maps[1]->GetDecalMode() == DECAL_M::re_no)
				return glm::normalize(TBN * tangent_normal);
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			Ray inverse_ray;
			glm::mat4 inverse_transform = *m_inv_transform;
			if (m_motion_blur != glm::vec3(0, 0, 0))
			{
				inverse_transform = *m_inv_transform * glm::inverse(glm::translate(glm::mat4(1.0f), ray.jitter_t * m_motion_blur)); //TODO
			}
			//inverse_transform = glm::inverse(inverse_transform);

			inverse_ray.direction = inverse_transform * glm::vec4(ray.direction, 0.0f);
			inverse_ray.origin = inverse_transform * glm::vec4(ray.origin, 1.0f);

			data->t = std::numeric_limits<float>().max();

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
			if (u < 0 || u > 1) return data->hit = false;

			glm::vec3 qvec = glm::cross(tvec, v0v1);
			float v = glm::dot(inverse_ray.direction, (qvec)) * invDet;
			if (v < 0 || u + v > 1) return data->hit = false;

			float t = glm::dot(v0v2, qvec) * invDet;

			if (t < inverse_ray.intersect_eps) return data->hit = false;

			bool smooth_normals = m_shading_mode == SHADING_MODE::smooth;
			bool replace_normals = false;
			if (m_tex_maps[1])
				replace_normals = true;

			glm::vec3 normal = smooth_normals ?
				(u * (*m_normals[1]) + v * (*m_normals[2]) + (1 - u - v) * (*m_normals[0])) :	// Smooth normal
				(glm::cross(v0v1, v0v2));														// Flat normal
			//normal = glm::normalize(normal);

			data->t = t;
			data->position = ray.PointAt(t);
			data->material = m_material.get();
			data->tex_map = m_tex_maps[0].get();
			if(m_uvs[1])
				data->uv = u * (*m_uvs[1]) + v * (*m_uvs[2]) + (1 - u - v) * (*m_uvs[0]);
			data->normal = glm::normalize(glm::mat3(glm::transpose(inverse_transform)) *
				(replace_normals ?
					(ObjectSpaceNormalAt(inverse_ray.PointAt(t), normal, { u,v }))	//BumpMap & NormalMap
					: normal));														// Regular normal

			return data->hit;
		}
	};

	class Sphere : public Shape
	{
	public:
		Sphere(std::shared_ptr<Material> mat, bool visible = true)
			:Shape(mat, visible)
		{
			m_shape_type = SHAPE_T::sphere;
		}

		Bounds3 GetWorldBounds() const
		{
			glm::vec3 r = 1.0f * glm::vec3(1.0f, 1.0f, 1.0f);
			glm::mat4 tr = *m_transform;

			glm::vec3 box[8] = {
				{1,1,1},
				{1,1,-1},
				{-1,1,-1},
				{-1,-1,-1},
				{-1,-1,1},
				{1,-1,1},
				{1,-1,-1} };

			glm::vec3 b_min = tr * glm::vec4(box[0], 1);
			glm::vec3 b_max = tr * glm::vec4(box[4], 1);

			for (int i = 0; i < 8; i++)
			{
				glm::vec3 tmp = tr * glm::vec4(box[i], 1);
				b_min = glm::min(b_min, tmp);
				b_max = glm::max(b_max, tmp);
			}

			b_min = glm::min(b_min, b_min + m_motion_blur);
			b_max = glm::max(b_max, b_max + m_motion_blur);


			return Bounds3(b_min, b_max);
		}

		Bounds3 GetLocalBounds() const
		{
			glm::vec3 t1 = 1.732050f * glm::vec3(1.0f, 1.0f, 1.0f);

			glm::vec3 t2 = 1.732050f * glm::vec3(1.0f, 1.0f, 1.0f);

			glm::vec3 b_min = glm::min(t1, t2); //glm::vec3(1.732050, 1.732050, 1.732050);
			glm::vec3 b_max = glm::max(t1, t2);//glm::vec3(1.732050, 1.732050, 1.732050);

			
			return Bounds3(b_min, b_max);
		}

		glm::vec3 ObjectSpaceNormalAt(glm::vec3 p, glm::vec3 normal, glm::vec2 uv) const
		{
			float pi = glm::pi<float>();
			float theta = acos(p.y); //[-pi/2,pi/2]
			float phi = atan2(p.z, p.x); //[-pi,+pi]
			glm::vec3 T, B, N;

			T = { p.z * 2 * pi, 0, -p.x * 2 * pi };
			B = { p.y * cos(phi) * pi, -sin(theta) * pi, p.y * sin(phi) * pi };
			N = glm::cross(T, B); N = theta >= 0 ? -N : N;

			glm::mat3 TBN = { glm::normalize(T), glm::normalize(B), glm::normalize(N) };

			glm::vec3 tangent_normal;
			if (m_tex_maps[1]->GetDecalMode() == DECAL_M::bump)
			{
				if (m_tex_maps[1]->GetType() == SOURCE_T::noise)
				{
					glm::vec3 g = m_tex_maps[1]->BumpAt( *m_transform* glm::vec4(p,1));
					return normal - glm::normalize(g - (glm::dot(g, normal) * normal) );
				}
				else //Image
				{
					glm::vec2 dudv = m_tex_maps[1]->BumpAt(glm::vec3(uv.x, uv.y, NAN));
					auto normal_prime = glm::normalize(normal - dudv.x * T - dudv.y * B);
					return abs(glm::dot(normal, normal_prime)) < 10e-6 ? -normal_prime : normal_prime;
				}
			}
			else
			{
				tangent_normal = glm::normalize( m_tex_maps[1]->SampleAt(glm::vec3(uv.x, uv.y, NAN)));
				return glm::normalize(TBN * tangent_normal);
			}
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			Ray inverse_ray;
			glm::mat4 inverse_transform = *m_inv_transform;
			if (m_motion_blur != glm::vec3(0, 0, 0))
			{
				inverse_transform *= glm::inverse(glm::translate(glm::mat4(1.0f), ray.jitter_t * m_motion_blur));
			}
			//glm::mat4 inverse_transform = glm::inverse(f_transform);
			inverse_ray.direction = inverse_transform * glm::vec4(ray.direction, 0.0f);
			inverse_ray.origin = inverse_transform * glm::vec4(ray.origin, 1.0f);
			inverse_ray.direction = glm::normalize(inverse_ray.direction);

			float t0, t1;

			float a = glm::dot(inverse_ray.direction, inverse_ray.direction);
			float b = 2.0f * glm::dot(inverse_ray.direction, (inverse_ray.origin ));
			float c = glm::dot(inverse_ray.origin, inverse_ray.origin) - 1.0f;

			double discr = b * b - 4.0 * a * c;
			if (discr < ray.intersect_eps)
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
			if (t0 < 0.0f) {
				t0 = t1; // if t0 is negative, let's use t1 instead 
				if (t0 < 0.0f) data->hit = false; // both t0 and t1 are negative 
			}

			bool replace_normals = false;
			if (m_tex_maps[1])
				replace_normals = true;

			glm::vec3 local_p = inverse_ray.PointAt(t0);

			data->t = glm::distance(glm::vec3( glm::inverse(inverse_transform) * glm::vec4(local_p,1.0f)), ray.origin);
			data->position = ray.PointAt(data->t);
			data->material = m_material.get();
			data->tex_map = m_tex_maps[0].get();
			data->uv = glm::vec2( (glm::pi<float>() - glm::atan(local_p.z, local_p.x)) / (2*glm::pi<float>()),
				acos(local_p.y) / glm::pi<float>());

			data->normal = glm::normalize( glm::mat3(glm::transpose(inverse_transform)) *
				(replace_normals ?
					ObjectSpaceNormalAt(local_p, local_p, data->uv) :		//Bump Map & Normal Map
					local_p));												//Regular Normal
			return data->hit;
		}
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
			return base_bounds;
		}

		glm::vec3 ObjectSpaceNormalAt(glm::vec3 p, glm::vec3 normal, glm::vec2 uv) const
		{
			return glm::vec3();
		}

		bool Intersect(const Ray ray, IntersectionData* data) const
		{
			bool hit = false;
			glm::mat4 inverse_transform = *m_inv_transform;
			if (m_motion_blur != glm::vec3(0, 0, 0))
			{
				inverse_transform = *m_inv_transform * glm::inverse(glm::translate(glm::mat4(1.0f), ray.jitter_t * m_motion_blur));
			}
			//inverse_transform = glm::inverse(inverse_transform);
			Ray inv_ray;
			inv_ray.origin = *(m_base_ptr->m_transform) * inverse_transform * glm::vec4(ray.origin, 1.0f);
			inv_ray.direction = *(m_base_ptr->m_transform) * inverse_transform * glm::vec4(ray.direction, 0.0f);
			inv_ray.intersect_eps = ray.intersect_eps;
			inv_ray.jitter_t = 0.0f;

			if (hit = m_base_ptr->Intersect(inv_ray, data))
			{
				data->normal = glm::normalize(glm::mat3(glm::transpose(inverse_transform)) *
					glm::inverse(glm::mat3(glm::transpose(glm::inverse(*(m_base_ptr->m_transform))))) * data->normal);
				data->position = ray.PointAt(data->t);
				if (m_material.get())
					data->material = m_material.get();
				if(m_tex_maps[0].get())
					data->tex_map = m_tex_maps[0].get();
			}
			return hit;
		}

	private:
		const Shape* m_base_ptr;
		bool m_reset_transform = false;
	};
}