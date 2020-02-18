#include "RayTracer.h"
#include <thirdparty/glm/glm/glm.hpp>
#include <limits>

namespace Chroma
{

#define T_MAX 100000;

	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_resolution.x, m_resolution.y);
	}


	void RayTracer::Render(Camera* cam, Scene& scene)
	{
		m_intersect_eps = scene.m_intersect_eps;
		glm::vec3 cam_pos = cam->GetPosition();
		glm::vec2 top_left = cam->GetNearPlane()[0];
		glm::vec2 bottom_right = cam->GetNearPlane()[1];
		float dist = cam->GetNearDist();

		glm::vec3 up = glm::normalize(cam->GetUp());
		glm::vec3 forward = glm::normalize(cam->GetGaze());
		glm::vec3 down = -up;
		glm::vec3 right = glm::normalize(glm::cross(forward, up));
		glm::vec3 left = -right;

		const glm::vec3 top_left_w = cam_pos + forward * dist + up * top_left.y + left * glm::abs(top_left.x);

		Ray camera_ray(cam_pos);

		const glm::vec3 right_step = (right) * glm::abs(top_left.x-bottom_right.x) / (float)m_resolution.x;
		const glm::vec3 down_step = (down) *glm::abs(top_left.y - bottom_right.y) / (float)m_resolution.y;

		for (int i = 0; i < m_resolution.x; i++)
		{
			for (int j = 0; j < m_resolution.y; j++)
			{
				glm::vec3 color = scene.m_sky_color;

				camera_ray.direction = glm::normalize(top_left_w + right_step * (i + 0.5f) + down_step * (j + 0.5f) - camera_ray.origin);

				//Go over scene objects and lights
				float t_min = std::numeric_limits<float>::max();
				std::map<std::string, std::shared_ptr<SceneObject>>::iterator it;
				for (it = scene.m_scene_objects.begin(); it != scene.m_scene_objects.end(); it++)
				{
					IntersectionData* intersection_data = new IntersectionData();
					if (it->second->IsVisible() && Intersect((it->second.get()), camera_ray, intersection_data)
						&& (glm::distance(camera_ray.origin, intersection_data->position) < t_min))//Hit
					{
						color = {0,0,0};
						t_min = glm::distance(camera_ray.origin, intersection_data->position);
						//lighting calculation
						std::map<std::string, std::shared_ptr<PointLight>>::iterator it2;
						for (it2 = scene.m_point_lights.begin(); it2 != scene.m_point_lights.end(); it2++)
						{
							std::shared_ptr<PointLight> pl = it2->second;
							glm::vec3 e_vec = camera_ray.origin - pl->position;
							glm::vec3 l_vec = pl->position - intersection_data->position;

							//Shadow calculation
							bool shadowed = false;
							IntersectionData* shd = new IntersectionData();
							for (auto it3 = scene.m_scene_objects.begin(); !shadowed && it3 != scene.m_scene_objects.end() && m_calc_shdws; it3++)
							{
								//if (it == it3) it3++;

								Ray shadow_ray(intersection_data->position + l_vec * scene.m_shadow_eps);
								shadow_ray.direction = glm::normalize(pl->position - shadow_ray.origin);
								shadowed = Intersect(it3->second.get(), shadow_ray, shd) &&
									glm::distance(shd->position, intersection_data->position) < glm::distance(intersection_data->position, pl->position);
								//CH_TRACE(std::string(it3->first + std::string(" is shadowed: ") + std::to_string(shadowed)));
							}
							delete shd;

							if(!shadowed)
							{
								float d = glm::distance(pl->position, intersection_data->position);

								//Kd * I * cos(theta) /d^2 
								glm::vec3 diffuse = intersection_data->material->diffuse * pl->intensity *
									glm::max(glm::dot(intersection_data->normal, l_vec), 0.0f)/  (glm::length(intersection_data->normal) * glm::length(l_vec))/(d*d);
								//Ks* I * max(0, r . dir) / d^2
								glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
								glm::vec3 specular = intersection_data->material->specular * pl->intensity *
									glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(intersection_data->normal))), it->second->GetMaterial()->shininess) / (d * d);
									//glm::pow(glm::max(0.0f, glm::dot(r, camera_ray.direction)), it->second->GetMaterial()->shininess) / (d*d);
								//specular = (_isnan(specular.x) || _isnan(specular.y) || _isnan(specular.z)) ? glm::vec3({0, 0, 0}) : specular;
								color += specular + diffuse;
							}
						}
						//Ka * Ia
						glm::vec3 ambient = scene.m_ambient_l * intersection_data->material->ambient;
						color += ambient;
					}
					delete intersection_data;
				}
				m_rendered_image->SetPixel(i, j, glm::clamp(color, 0.0f, 255.0f));
			}
			CH_TRACE(std::to_string(((float)i) / ((float)m_resolution.x) * 100.0f) + std::string("% complete"));
		}
		CH_TRACE("Rendered");
	}

	void RayTracer::SetResoultion(const glm::ivec2& resolution)
	{
		if (m_resolution == resolution)
			return;

		m_resolution = resolution;

		delete m_rendered_image;

		m_rendered_image = new Image(m_resolution.x, m_resolution.y);
	}

	bool RayTracer::Intersect(SceneObject* obj, Ray ray, IntersectionData* intersection_data)
	{
		//SPHERE INTERSECTION
		if (obj->GetRTIntersectionMethod() == RT_INTR_METHOD::sphere)
		{
			float a = glm::dot(ray.direction, ray.direction);
			float b = 2.0f * glm::dot(ray.direction, (ray.origin - obj->GetPosition()));
			float c = glm::dot(ray.origin - obj->GetPosition(), ray.origin - obj->GetPosition()) - obj->m_radius * obj->m_radius;

			float t0, t1;
			double discr = b * b - 4 * a * c;
			if (discr < 0) 
				return false;
			else if (discr == 0) //single root
				t0 = t1 = -0.5 * b / a;
			else {
				float q = (b > 0) ?
					-0.5 * (b + (double)glm::sqrt(discr)) : -0.5 * (b - (double)glm::sqrt(discr));
				t0 = q / a;
				t1 = c / q;
			}
			if (t0 > t1) 
				std::swap(t0, t1);

			if (t0 < 0) {
				t0 = t1; // if t0 is negative, let's use t1 instead 
				if (t0 < 0) return false; // both t0 and t1 are negative 
			}

			intersection_data->hit = discr >= m_intersect_eps;
			intersection_data->material = obj->GetMaterial();
			intersection_data->position = ray.PointAt(t0);
			intersection_data->normal = glm::normalize(intersection_data->position - obj->GetPosition());
			intersection_data->uv = glm::vec2(glm::atan(intersection_data->position.z, intersection_data->position.x),
				glm::acos(intersection_data->position.y / obj->m_radius));

			return intersection_data->hit;
		}
		//TRIANGLE INTERSECTION
		else if (obj->GetRTIntersectionMethod() == RT_INTR_METHOD::triangle)
		{
			std::vector<glm::vec3*> verts;
			std::vector<glm::vec3*> norms;

			verts.reserve(3);
			verts.resize(3);
			verts[0] = &obj->m_mesh.m_vertex_positions[obj->m_mesh.m_indices[0]];
			verts[1] = &obj->m_mesh.m_vertex_positions[obj->m_mesh.m_indices[1]];
			verts[2] = &obj->m_mesh.m_vertex_positions[obj->m_mesh.m_indices[2]];

			norms.reserve(3);
			norms.resize(3);
			norms[0] = &obj->m_mesh.m_vertex_normals[obj->m_mesh.m_indices[0]];
			norms[1] = &obj->m_mesh.m_vertex_normals[obj->m_mesh.m_indices[1]];
			norms[2] = &obj->m_mesh.m_vertex_normals[obj->m_mesh.m_indices[2]];

			if (IntersectTriangle(verts, norms, ray, intersection_data))
			{
				intersection_data->material = obj->GetMaterial();
				return intersection_data->hit;
			}
		}
		//MESH INTERSECTION (multiple tringles)
		else if (obj->GetRTIntersectionMethod() == RT_INTR_METHOD::mesh)
		{
			std::vector<glm::vec3*> verts;
			verts.reserve(3);
			verts.resize(3);

			std::vector<glm::vec3*> norms;
			norms.reserve(3);
			norms.resize(3);

			for (int i = 0; i < obj->m_mesh.GetFaceCount(); i++) 
			{
				verts[0] = &obj->m_mesh.m_vertex_positions[obj->m_mesh.m_indices[i * 3]];
				verts[1] = &obj->m_mesh.m_vertex_positions[obj->m_mesh.m_indices[i * 3 + 1]];
				verts[2] = &obj->m_mesh.m_vertex_positions[obj->m_mesh.m_indices[i * 3 + 2]];
				norms[0] = &obj->m_mesh.m_vertex_normals[obj->m_mesh.m_indices[i * 3]];
				norms[1] = &obj->m_mesh.m_vertex_normals[obj->m_mesh.m_indices[i * 3 + 1]];
				norms[2] = &obj->m_mesh.m_vertex_normals[obj->m_mesh.m_indices[i * 3 + 2]];

				if (IntersectTriangle(verts, norms, ray, intersection_data))
				{
					intersection_data->material = obj->GetMaterial();
					return intersection_data->hit;
				}
			}
		}
		return false;
	}

	bool RayTracer::IntersectTriangle(std::vector<glm::vec3*> vertices, std::vector<glm::vec3*> normals, Ray ray, IntersectionData* intersection_data)
	{
		if (vertices.size() != 3)
		{
			CH_WARN("Non triangle object fed into intersetion method");
			return false;
		}
		glm::vec3 v0 = *vertices[0];
		glm::vec3 v1 = *vertices[1];
		glm::vec3 v2 = *vertices[2];

		glm::vec3 v0v1 = v1 - v0;
		glm::vec3 v0v2 = v2 - v0;

		glm::vec3 pvec = glm::cross(ray.direction, v0v2);
		float det = glm::dot(v0v1, pvec);

		intersection_data->hit = true;
		if ((det) < m_intersect_eps) intersection_data->hit = false;

		float invDet = 1 / det;

		glm::vec3 tvec = ray.origin - v0;
		float u = glm::dot(tvec, (pvec)) * invDet;
		if (u < 0 || u > 1) intersection_data->hit = false;

		glm::vec3 qvec = glm::cross(tvec, v0v1);
		float v = glm::dot(ray.direction, (qvec)) * invDet;
		if (v < 0 || u + v > 1) intersection_data->hit = false;

		float t = glm::dot(v0v2, qvec) * invDet;

		if (t < m_intersect_eps) return false;

		intersection_data->position = ray.PointAt(t);
		intersection_data->normal = glm::cross(v0v1, v0v2); //u *(*normals[1]) + v * (*normals[2]) + (1 - u - v) * (*normals[0]);

		return intersection_data->hit;

		return false;
	}

}
