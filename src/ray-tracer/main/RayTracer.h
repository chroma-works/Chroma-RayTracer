#pragma once

#include <ray-tracer/main/Image.h>
#include <ray-tracer/main/Ray.h>
#include <ray-tracer/main/Scene.h>
#include <ray-tracer/editor/Settings.h>

namespace Chroma
{
	enum RT_MODE{ray_cast=0, path_trace, size};
	class RayTracer
	{
	public:
		RayTracer();

		void Render(Camera* cam, Scene& scene);
		void SetResoultion(const glm::ivec2& resolution);

		void SetRenderMode(RT_MODE mode);

	private:
		friend class Editor;
		Image* m_rendered_image ;

		Settings m_settings;

		void(RayTracer::* m_rt_mode)(Camera* cam, Scene& scene, int idx);

		void RayCastWorker(Camera* cam, Scene& scene, int idx);
	};
}
