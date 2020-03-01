#include "RayTracer.h"
#include <thirdparty/glm/glm/glm.hpp>
#include <limits>

namespace Chroma
{
#define MAX_RAY_DEPTH 6 

	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_settings.resolution.x, m_settings.resolution.y);
		for (int i = 0; i < m_settings.resolution.x; i++)
			for (int j = 0; j < m_settings.resolution.y; j++)
				m_rendered_image->SetPixel(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
		m_rt_worker = &RayTracer::RayCastWorker;
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
