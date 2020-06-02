#pragma once

#include <ray-tracer/main/Image.h>
#include <ray-tracer/main/Ray.h>
#include <ray-tracer/main/Scene.h>
#include <ray-tracer/editor/Settings.h>

namespace CHR
{
	enum RT_MODE{ray_cast=0, recursive_trace, size};
	class RayTracer
	{
	public:
		RayTracer();

		void Render(Camera* cam, Scene& scene, bool print_progress = true);
		void SetResoultion(const glm::ivec2& resolution);

		void SetRenderMode(RT_MODE mode);


	private:
		friend class Editor;
		Image* m_rendered_image ;

		Settings m_settings;
		std::atomic<int> job_index{ 0 };

		void(RayTracer::* m_rt_worker)(Camera* cam, Scene& scene, int idx);

		void RayCastWorker(Camera* cam, Scene& scene, int idx);
		void RecursiveTraceWorker(Camera* cam, Scene& scene, int idx);
		bool TestShadow(const Scene& scene, const IntersectionData* isect_data, const std::shared_ptr<Light> l, const Ray shadow_ray);
		glm::vec3 RecursiveTrace(const Ray& ray, Scene& scene, int depth, glm::ivec2 pixel_cood);
	};
}
