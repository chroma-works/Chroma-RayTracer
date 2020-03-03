#include "RayTracer.h"
#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/gtx/string_cast.hpp>
#include <limits>
#include <thirdparty\glm\glm\gtx\norm.hpp>

namespace Chroma
{
#define MAX_RAY_DEPTH 6 

	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_settings.resolution.x, m_settings.resolution.y);
		for (int i = 0; i < m_settings.resolution.x; i++)
			for (int j = 0; j < m_settings.resolution.y; j++)
				m_rendered_image->SetPixel(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
		m_rt_worker = &RayTracer::PathTraceWorker;
	}

	std::atomic<float> progress_pers;

	void RayTracer::Render(Camera* cam, Scene& scene)
	{
		if (!scene.m_accel_structure)
		{
			return;
		}
		progress_pers = 0.0f;
		std::thread** threads = new std::thread * [m_settings.thread_count];

		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		for (int i = 0; i < m_settings.thread_count; i++)
			threads[i] = new std::thread(m_rt_worker, this, cam, std::ref(scene), i);

		for (int i = 0; i < m_settings.thread_count; i++)
		{
			threads[i]->join();
			delete threads[i];
		}

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		delete[] threads;

		std::chrono::duration<float> fs = end - start;
		std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds>(fs);


		unsigned int triangle_count = 0;

		for (auto it = scene.m_scene_objects.begin(); it != scene.m_scene_objects.end(); it++)
		{
			if(it->second->GetRTIntersectionMethod() != RT_INTR_TYPE::sphere)
				triangle_count += it->second->m_mesh.GetFaceCount();

			glm::vec3 min = it->second->m_mesh.GetMinBound();
			glm::vec3 max = it->second->m_mesh.GetMaxBound();
		}
		CH_TRACE("Render info:\n\tTriangles :" + std::to_string(triangle_count) +
			"\n\tResolution: (" + std::to_string(cam->m_res.x) + ", " + std::to_string(cam->m_res.y) +
			")\n\tRendered in " + std::to_string(fs.count()) + "s" 
			+ "\n\tThreads: " + std::to_string(m_settings.thread_count));
	}

	void RayTracer::RayCastWorker(Camera* cam, Scene& scene, int idx)
	{
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
		Ray primary_ray(cam_pos);

		const glm::vec3 right_step = (right)*glm::abs(top_left.x - bottom_right.x) / (float)m_settings.resolution.x;
		const glm::vec3 down_step = (down)*glm::abs(top_left.y - bottom_right.y) / (float)m_settings.resolution.y;

		IntersectionData* intersection_data = new IntersectionData();
		IntersectionData* shadow_data = new IntersectionData();

		int col_start = (float)idx / (float)m_settings.thread_count * m_settings.resolution.x;
		int col_end = idx == m_settings.thread_count -1 ? m_settings.resolution.x :
			(float)(idx + 1) / (float)m_settings.thread_count * m_settings.resolution.x;

		//CH_TRACE(std::string(std::to_string(col_start)) + "-" + std::to_string(col_end));

		for (int i = col_start; i < col_end; i++)
		{
			for (int j = 0; j < m_settings.resolution.y; j++)
			{
				glm::vec3 color = scene.m_sky_color;

				primary_ray.direction = glm::normalize(top_left_w + right_step * (i + 0.5f) + down_step * (j + 0.5f) - primary_ray.origin);
				//Go over scene objects and lights
				float t_min = std::numeric_limits<float>::max();
				if (scene.m_accel_structure->Intersect(primary_ray, scene.m_intersect_eps, intersection_data))//Hit
				{
					color = { 0,0,0 };
					//lighting calculation
					std::map<std::string, std::shared_ptr<PointLight>>::iterator it2;
					for (it2 = scene.m_point_lights.begin(); it2 != scene.m_point_lights.end(); it2++)
					{
						std::shared_ptr<PointLight> pl = it2->second;
						glm::vec3 e_vec = glm::normalize(primary_ray.origin - intersection_data->position);
						glm::vec3 l_vec = glm::normalize(pl->position - intersection_data->position);

						//Shadow calculation	
						bool shadowed = false;
						Ray shadow_ray(intersection_data->position + intersection_data->normal * scene.m_shadow_eps);
						shadow_ray.direction = glm::normalize(pl->position - shadow_ray.origin);
							
						shadowed = m_settings.calc_shadows && (scene.m_accel_structure->Intersect(shadow_ray, scene.m_intersect_eps, shadow_data) &&
							shadow_data->t < glm::distance(intersection_data->position, pl->position));

						if (!shadowed)
						{
							float d = glm::distance(pl->position, intersection_data->position);

							//Kd * I * cos(theta) /d^2 
							glm::vec3 diffuse = intersection_data->material->diffuse * pl->intensity *
								glm::max(glm::dot(intersection_data->normal, l_vec), 0.0f) / (glm::length(intersection_data->normal) * glm::length(l_vec)) / (d * d);
							//Ks* I * max(0, h . n)^s / d^2
							glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
							glm::vec3 specular = intersection_data->material->specular * pl->intensity *
								glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(intersection_data->normal))), intersection_data->material->shininess) / (d * d);
							color += specular + diffuse;
						}
					}
					//Ka * Ia
					glm::vec3 ambient = scene.m_ambient_l * intersection_data->material->ambient;
					color += ambient;
				}
				m_rendered_image->SetPixel(i, j, glm::clamp(color, 0.0f, 255.0f));
			}
			progress_pers = progress_pers + (1.0f) / ((float)(m_settings.resolution.x));
			if(idx == m_settings.thread_count - 1)
				CH_TRACE(std::to_string(progress_pers * 100.0f) + std::string("% complete"));
		}
		delete intersection_data;
		delete shadow_data;
	}

	void RayTracer::PathTraceWorker(Camera* cam, Scene& scene, int idx)
	{
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
		Ray primary_ray(cam_pos);

		const glm::vec3 right_step = (right)*glm::abs(top_left.x - bottom_right.x) / (float)m_settings.resolution.x;
		const glm::vec3 down_step = (down)*glm::abs(top_left.y - bottom_right.y) / (float)m_settings.resolution.y;

		IntersectionData* intersection_data = new IntersectionData();
		IntersectionData* shadow_data = new IntersectionData();

		int col_start = (float)idx / (float)m_settings.thread_count * m_settings.resolution.x;
		int col_end = idx == m_settings.thread_count - 1 ? m_settings.resolution.x :
			(float)(idx + 1) / (float)m_settings.thread_count * m_settings.resolution.x;

		glm::vec3 color = scene.m_sky_color;
		for (int i = col_start; i < col_end; i++)
		{
			for (int j = 0; j < m_settings.resolution.y; j++)
			{

				primary_ray.direction = glm::normalize(top_left_w + right_step * (i + 0.5f) + down_step * (j + 0.5f) - primary_ray.origin);
				color = PathTrace(primary_ray, scene, 0);

				m_rendered_image->SetPixel(i, j, glm::clamp(color, 0.0f, 255.0f));

			}

			progress_pers = progress_pers + (1.0f) / ((float)(m_settings.resolution.x));
			if (idx == m_settings.thread_count - 1)
				CH_TRACE(std::to_string(progress_pers * 100.0f) + std::string("% complete"));
		}
	}

	glm::vec3 RayTracer::PathTrace(const Ray& ray, Scene& scene, int depth)//Recursive!
	{
		IntersectionData* isect_data = new IntersectionData();
		scene.m_accel_structure->Intersect(ray, scene.m_intersect_eps, isect_data);

		glm::vec3 color= { 0,0,0 };

		if (!isect_data->hit)
		{
			delete isect_data;
			return scene.m_sky_color;
		}

		else if (isect_data->material->type == MAT_TYPE::mirror && depth < MAX_RAY_DEPTH) {
			// compute reflection
			Ray reflection_ray(isect_data->position + isect_data->normal * m_settings.shadow_eps);
			reflection_ray.direction = glm::normalize(glm::reflect(ray.direction, isect_data->normal));

			glm::vec3 reflection_color = PathTrace(reflection_ray, scene, depth + 1) * isect_data->material->f_coeff.conductor_coeffs.mirror_reflec;
			color += reflection_color;
		}
		else if (isect_data->material->type == MAT_TYPE::conductor && depth < MAX_RAY_DEPTH)
		{
			// compute reflection
			Ray reflection_ray(isect_data->position + isect_data->normal * m_settings.shadow_eps);
			reflection_ray.direction = glm::normalize(glm::reflect(ray.direction, isect_data->normal) );

			float cos_theta = glm::dot(-ray.direction, isect_data->normal);// /1*1

			glm::vec3 reflection_color = PathTrace(reflection_ray, scene, depth + 1) * isect_data->material->GetFr(cos_theta) * 
				isect_data->material->f_coeff.conductor_coeffs.mirror_reflec;
			color += reflection_color;
		}
		else if (isect_data->material->type == MAT_TYPE::dielectric && depth < MAX_RAY_DEPTH)
		{
			float cos_i = glm::clamp(glm::dot(ray.direction, isect_data->normal),-1.0f,1.0f);
			float ni = 1.0f;
			float nt = isect_data->material->f_coeff.dielectric_coeffs.refraction_ind;

			glm::vec3 proper_normal = isect_data->normal;

			bool inside = (cos_i > 0.0f);

			if (inside)
			{
				std::swap(ni, nt);
				proper_normal = -isect_data->normal;
			}
			/*
			float sin2_t = ni / nt * sin_t;
			float cos_t = std::sqrt(std::max(0.0f, 1.0f - sin_t * sin_t));

			float Rparl = ((nt * cos_i) - (ni * cos_t)) /
				((nt * cos_i) + (ni* cos_t));
			float Rperp = ((ni * cos_i) - (nt * cos_t)) /
				((ni * cos_i) + (nt * cos_t));

			float fr = (Rparl * Rparl + Rperp * Rperp) * 0.5f;*/

			float fr = isect_data->material->GetFr(cos_i);

			glm::vec3 uv = glm::normalize(ray.direction);
			float dt = dot(uv, proper_normal);
			float int_ref = glm::length2(ni / nt * (glm::dot(ray.direction, -proper_normal) * -proper_normal - ray.direction));
			cos_i = std::abs(cos_i);

			/*float sin_i = std::sqrt(std::max(0.0f, 1.0f - cos_i * cos_i));
			float sin_t = ni / nt * sin_i;*/
			float sin2_i = std::max(0.f, 1.f - cos_i * cos_i);
			float sin_t = ni/nt * ni/nt * sin2_i;
			//fr = (sin_t > 1.0f) ? 1.0f : fr;
			fr = (sin_t >= 1.0f) ? 1.0f : fr;
			//fr = discriminat > 0.0f ? fr : 1.0f;

			Ray reflection_ray(isect_data->position + proper_normal * m_settings.shadow_eps);
			reflection_ray.direction = glm::normalize(glm::reflect(ray.direction, proper_normal));

			glm::vec3 reflection_color = PathTrace(reflection_ray, scene, depth + 1) * fr;
			if (inside)
			{
				reflection_color *= exp(-isect_data->material->f_coeff.dielectric_coeffs.absorption_coeff * glm::distance(ray.origin, isect_data->position));
			}


			glm::vec3 refraction_color = { 0,0,0 };

			if (fr < 1.0f)
			{
				Ray refraction_ray(isect_data->position - proper_normal * m_settings.shadow_eps);
				refraction_ray.direction = glm::normalize(glm::refract(ray.direction, proper_normal, ni / nt));
				refraction_color = PathTrace(refraction_ray, scene, depth + 1) * (1.0f - fr);
				if (!inside)
				{
					refraction_color *= exp(-isect_data->material->f_coeff.dielectric_coeffs.absorption_coeff * glm::distance(ray.origin, isect_data->position));
				}
			}
			/*else if (inside)
			{
				//CH_TRACE(glm::to_string(exp(-isect_data->material->f_coeff.dielectric_coeffs.absorption_coeff * glm::distance(ray.origin, isect_data->position))));
				reflection_color *= exp(-isect_data->material->f_coeff.dielectric_coeffs.absorption_coeff * glm::distance(ray.origin, isect_data->position));
			}
			if (!inside)
			{
				refraction_color *= exp(-isect_data->material->f_coeff.dielectric_coeffs.absorption_coeff * glm::distance(ray.origin, isect_data->position));
			}*/

			color += (reflection_color + refraction_color);
		}

		// point is illuminated
		if (isect_data->hit )
		{
			IntersectionData* shadow_data = new IntersectionData();
			//lighting calculation
			for (auto it = scene.m_point_lights.begin(); it != scene.m_point_lights.end(); it++)
			{
				std::shared_ptr<PointLight> pl = it->second;
				glm::vec3 e_vec = glm::normalize(ray.origin - isect_data->position);
				glm::vec3 l_vec = glm::normalize(pl->position - isect_data->position);

				//Shadow calculation	
				bool shadowed = false;
				Ray shadow_ray(isect_data->position + isect_data->normal * scene.m_shadow_eps);
				shadow_ray.direction = glm::normalize(pl->position - shadow_ray.origin);

				shadowed = m_settings.calc_shadows && (scene.m_accel_structure->Intersect(shadow_ray, scene.m_intersect_eps, shadow_data) &&
					shadow_data->t < glm::distance(isect_data->position, pl->position));

				if (!shadowed)
				{
					float d = glm::distance(pl->position, isect_data->position);

					//Kd * I * cos(theta) /d^2 
					glm::vec3 diffuse = isect_data->material->diffuse * pl->intensity *
						glm::max(glm::dot(isect_data->normal, l_vec), 0.0f) / (glm::length(isect_data->normal) * glm::length(l_vec)) / (d * d);
					//Ks* I * max(0, h . n)^s / d^2
					glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
					glm::vec3 specular = isect_data->material->specular * pl->intensity *
						glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_data->normal))), isect_data->material->shininess) / (d * d);
					color += specular + diffuse;
				}
			}
			delete shadow_data;
			//Ka * Ia
			glm::vec3 ambient = scene.m_ambient_l * isect_data->material->ambient;
			color += ambient;
		}
		delete isect_data;
		return color;
	}

	void RayTracer::SetResoultion(const glm::ivec2& resolution)
	{
		if (m_settings.resolution == resolution)
			return;

		m_settings.resolution = resolution;

		delete m_rendered_image;

		m_rendered_image = new Image(m_settings.resolution.x, m_settings.resolution.y);
		for (int i = 0; i < m_settings.resolution.x; i++)
			for (int j = 0; j < m_settings.resolution.y; j++)
				m_rendered_image->SetPixel(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	void RayTracer::SetRenderMode(RT_MODE mode)
	{
		switch (mode)
		{
		case Chroma::ray_cast:
			m_rt_worker = &RayTracer::RayCastWorker;
			break;
		case Chroma::path_trace:
			m_rt_worker = &RayTracer::PathTraceWorker;
			break;
		case Chroma::size:
			break;
		default:
			break;
		}
	}

}
