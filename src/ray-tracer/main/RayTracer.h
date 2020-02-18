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
		bool m_calc_shdws = false;
		float m_intersect_eps;

		Image* m_rendered_image;

		bool Intersect(SceneObject* obj, Ray ray, IntersectionData* intersection_data);
		bool IntersectTriangle(std::vector<glm::vec3> vertices, std::vector<glm::vec3*> normals, Ray ray, IntersectionData* intersection_dat);
	};
}
