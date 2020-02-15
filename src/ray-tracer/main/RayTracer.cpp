#include "RayTracer.h"

namespace Chroma
{
	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_resolution.x, m_resolution.y);

	}


	void RayTracer::Render(Camera* cam, Scene& scene)
	{
		for (int i = 0; i < m_resolution.x; i++)
		{
			for (int j = 0; j < m_resolution.y; j++)
			{
				glm::vec3 color = scene.m_sky_color;
				glm::vec3 cam_pos = cam->GetPosition();
				//Ray ray(glm::vec3(cam_pos - ), glm::vec3());

				m_rendered_image->SetPixel(i, j, color);
			}
		}
	}

	void RayTracer::SetResoultion(const glm::ivec2& resolution)
	{
		if (m_resolution == resolution)
			return;

		m_resolution = resolution;

		delete m_rendered_image;

		m_rendered_image = new Image(m_resolution.x, m_resolution.y);
	}

}
