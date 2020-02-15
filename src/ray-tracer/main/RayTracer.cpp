#include "RayTracer.h"

namespace Chroma
{
	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_resolution.x, m_resolution.y);

	}


	void RayTracer::Render(Scene& scene)
	{

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
