#pragma once
#include <ray-tracer/main/SceneObject.h>
//source: https://www.scratchapixel.com/code.php?id=36&origin=/lessons/advanced-rendering/introduction-acceleration-structure


namespace Chroma
{
	class AccelerationStructure
	{
	public:
		AccelerationStructure(std::vector<std::shared_ptr<SceneObject>>& m) : m_scene_objects(m) {}
		virtual ~AccelerationStructure() {}
		virtual bool Intersect(const Ray ray, float intersect_eps, IntersectionData* isect_data) const
		{
			float tHit = INFINITY;
			const SceneObject* intersected_obj = nullptr;
			IntersectionData* temp_data = new IntersectionData();
			for (auto& obj : m_scene_objects) {
				if (obj->IsVisible() && obj->Intersect(ray, intersect_eps, temp_data) && temp_data->t < tHit) {
					*isect_data = *temp_data;
					intersected_obj = obj.get();
					tHit = temp_data->t;
				}
			}
			delete temp_data;
			return (intersected_obj != nullptr);
		}
	protected:
		const std::vector<std::shared_ptr<SceneObject>> m_scene_objects;
	};
}