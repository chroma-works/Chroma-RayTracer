#pragma once
#include <ray-tracer/main/SceneObject.h>


namespace Chroma
{
	class AccelerationStructure
	{
	public:
		AccelerationStructure(std::vector<std::shared_ptr<SceneObject>>& m) : m_scene_objects(m) {}
		virtual ~AccelerationStructure() {}
		virtual bool Intersect(const Ray ray, const uint32_t& rayId, float& tHit) const
		{
			const SceneObject* intersectedMesh = nullptr;
			float t = INFINITY;
			IntersectionData* isect_data = new IntersectionData();
			for (auto& obj : m_scene_objects) {
				if (obj->Intersect(ray, 0.000001f, isect_data) && (t = glm::distance(isect_data->position, ray.origin) < tHit)) {
					intersectedMesh = obj.get();
					tHit = t;
				}
			}
			delete isect_data;
			return (intersectedMesh != nullptr);
		}
	protected:
		const std::vector<std::shared_ptr<SceneObject>> m_scene_objects;
	};
}