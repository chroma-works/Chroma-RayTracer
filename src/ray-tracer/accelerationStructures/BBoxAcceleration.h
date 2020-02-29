#pragma once
#include <ray-tracer/accelerationStructures/AccelerationStructure.h>
//source: https://www.scratchapixel.com/code.php?id=36&origin=/lessons/advanced-rendering/introduction-acceleration-structure

namespace Chroma
{
	class BBoxAcceleration : public AccelerationStructure
	{
	public:
		BBoxAcceleration(std::vector<std::shared_ptr<SceneObject>>& s) : AccelerationStructure(s) {}

		virtual bool intersect(const Ray ray, float intersect_eps, IntersectionData* isect_data) const
		{
			const SceneObject* intersected_obj = nullptr;
			float t = INFINITY;
			float tHit;
			IntersectionData* temp_data = new IntersectionData();
			for (auto& obj : m_scene_objects) {
				// If you intersect the box
				if (obj->IntersectBBox(ray, intersect_eps, t)) {
					// Then test if the ray intersects the mesh and if does then first check
					// if the intersection distance is the nearest and if we pass that test as well
					// then update tNear variable with t and keep a pointer to the intersected mesh
					if (obj->Intersect(ray, intersect_eps, temp_data) && temp_data->t < tHit) {
						intersected_obj = obj.get();
						tHit = temp_data->t;
						*isect_data = *temp_data;
					}
				}
			}
			delete temp_data;
			// Return true if the variable intersectedMesh is not null, false otherwise
			return (intersected_obj != nullptr);
		}
	};

}