#include "RayTracer.h"
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_resolution.x, m_resolution.y);

	}


	void RayTracer::Render(Camera* cam, Scene& scene)
	{
		glm::vec3 cam_pos = cam->GetPosition();
		glm::vec3 cam_gaze = glm::normalize(cam->GetGaze());
		glm::vec2 top_left = cam->GetNearPlane()[0];
		glm::vec2 bottom_right = cam->GetNearPlane()[1];
		float dist = cam->GetNearDist();
		glm::vec3 up_v = glm::normalize(cam->GetUp());
		glm::vec3 right_v = glm::normalize(glm::cross(cam_gaze, up_v));
		glm::vec3 top_left_world_coord = cam_pos + top_left.y * up_v + top_left.x * -right_v;
		CH_TRACE(top_left_world_coord.x);
		CH_TRACE(top_left_world_coord.y);
		CH_TRACE(top_left_world_coord.z);

		for (int i = 0; i < m_resolution.x; i++)
		{
			for (int j = 0; j < m_resolution.y; j++)
			{
				glm::vec3 color = scene.m_sky_color;



				/*float u = float(i) / float(m_resolution.x);
				float v = float(j) / float(m_resolution.y);*/

				float pw = float(top_left.x - bottom_right.x)/ float(m_resolution.x);
				float ph = float(top_left.y - bottom_right.y) / float(m_resolution.y);

				glm::vec3 q = top_left_world_coord + pw * (i + 0.5f) * right_v + ph * (j + 0.5f) * -up_v;

				Ray ray(cam_pos - cam_gaze * dist, glm::normalize(q));

				std::map<std::string, std::shared_ptr<SceneObject>>::iterator it;
				for (it = scene.m_scene_objects.begin(); it != scene.m_scene_objects.end(); it++)
				{
					if (intersect(*(it->second.get()), ray))
					{
						CH_TRACE("HIT!");
						color += glm::vec3(255, 255, 255);
					}
				}

				m_rendered_image->SetPixel(i, j, color);
			}
			CH_TRACE(i);
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

	bool RayTracer::intersect(SceneObject obj, Ray ray)
	{
		//SPHERE INTERSECTION
		if (obj.GetRTIntersectionMethod() == RT_INTR_METHOD::sphere)
		{
			float a = glm::dot(ray.direction, ray.direction);
			float b = 2.0f * glm::dot(ray.direction, (ray.origin - obj.GetPosition()));
			float c = glm::dot(ray.origin - obj.GetPosition(), ray.origin - obj.GetPosition()) - obj.m_radius * obj.m_radius;

			//float t1 = - b - glm::sqrt(b * b - 4 * a * c)/(2.0f * a);
			return b * b - 4 * a * c > 0;
		}
		return false;
	}

}
