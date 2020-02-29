#pragma once

#include <ray-tracer/main/Image.h>
#include <ray-tracer/main/Ray.h>
#include <ray-tracer/main/Scene.h>
#include <ray-tracer/editor/Settings.h>

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
		Image* m_rendered_image ;

		Settings m_settings;

		void RayTraceWorker(Camera* cam, Scene& scene, int idx);
	};
}
