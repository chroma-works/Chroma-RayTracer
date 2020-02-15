#pragma once

#include <ray-tracer/main/Image.h>
#include <ray-tracer/main/Ray.h>
#include <ray-tracer/main/Scene.h>

namespace Chroma
{
	class RayTracer
	{
	public:
		RayTracer();

		void Render(Camera* cam, Scene& scene);
		void SetResoultion(const glm::ivec2& resolution);

	private:
		friend class Editor;
		glm::ivec2 m_resolution = { 800,800 };

		Image* m_rendered_image;

		bool intersect(SceneObject obj, Ray ray);
	};
}
