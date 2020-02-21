#include "RayTracer.h"
#include <thirdparty/glm/glm/glm.hpp>
#include <limits>

namespace Chroma
{

#define T_MAX 100000;

	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_resolution.x, m_resolution.y);
		for (int i = 0; i < m_resolution.x; i++)
			for (int j = 0; j < m_resolution.y; j++)
				m_rendered_image->SetPixel(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
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

		IntersectionData* intersection_data = new IntersectionData();
		IntersectionData* shadow_data = new IntersectionData();

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
					if (it->second->IsVisible() && it->second->Intersect(camera_ray, m_intersect_eps, intersection_data)
						&& (glm::distance(camera_ray.origin, intersection_data->position) < t_min))//Hit
					{
						color = {0,0,0};
						t_min = glm::distance(camera_ray.origin, intersection_data->position);
						//lighting calculation
						std::map<std::string, std::shared_ptr<PointLight>>::iterator it2;
						for (it2 = scene.m_point_lights.begin(); it2 != scene.m_point_lights.end(); it2++)
						{
							std::shared_ptr<PointLight> pl = it2->second;
							glm::vec3 e_vec = glm::normalize(camera_ray.origin - intersection_data->position);
							glm::vec3 l_vec = glm::normalize(pl->position - intersection_data->position);

							//Shadow calculation
							bool shadowed = false;
							for (auto it3 = scene.m_scene_objects.begin(); !shadowed && it3 != scene.m_scene_objects.end() && m_calc_shdws; it3++)
							{
								//if (it == it3) it3++;

								Ray shadow_ray(intersection_data->position + l_vec * scene.m_shadow_eps);
								shadow_ray.direction = glm::normalize(pl->position - shadow_ray.origin);
								shadowed = it3->second->IsVisible() && it3->second->Intersect(shadow_ray, m_intersect_eps, shadow_data) &&
									glm::distance(shadow_data->position, intersection_data->position) < glm::distance(intersection_data->position, pl->position);
								//CH_TRACE(std::string(it3->first + std::string(" is shadowed: ") + std::to_string(shadowed)));
							}

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
				}
				m_rendered_image->SetPixel(i, j, glm::clamp(color, 0.0f, 255.0f));
			}
			CH_TRACE(std::to_string(((float)i) / ((float)m_resolution.x) * 100.0f) + std::string("% complete"));
		}
		delete intersection_data;
		delete shadow_data;
		CH_TRACE("Rendered");
	}

	void RayTracer::SetResoultion(const glm::ivec2& resolution)
	{
		if (m_resolution == resolution)
			return;

		m_resolution = resolution;

		delete m_rendered_image;

		m_rendered_image = new Image(m_resolution.x, m_resolution.y);
		for (int i = 0; i < m_resolution.x; i++)
			for (int j = 0; j < m_resolution.y; j++)
				m_rendered_image->SetPixel(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
	}

}
