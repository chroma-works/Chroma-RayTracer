#include "BVH.h"
#include <cmath>
#include <ray-tracer/main/SceneObject.h>
#include <stdio.h>
#include <thirdparty/glm/glm/gtx/string_cast.hpp>
#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/gtx/rotate_vector.hpp>
#include <thirdparty/glm/glm/gtx/euler_angles.hpp>
#include <queue>
//source: https://www.scratchapixel.com/code.php?id=36&origin=/lessons/advanced-rendering/introduction-acceleration-structure


namespace Chroma
{
	const glm::vec3 BVH::planeSetNormals[BVH::kNumPlaneSetNormals] = {
	glm::vec3(1, 0, 0),
	glm::vec3(0, 1, 0),
	glm::vec3(0, 0, 1),
	glm::vec3(sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::vec3(-sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::vec3(-sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::vec3(sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f)
	};

	BVH::BVH(std::vector<std::shared_ptr<SceneObject>>& s) : AccelerationStructure(s)
	{
		Extents sceneExtents; // that's the extent of the entire scene which we need to compute for the octree 
		extentsList.reserve(m_scene_objects.size());
		extentsList.resize(m_scene_objects.size());
		for (uint32_t i = 0; i < m_scene_objects.size(); ++i) {
			for (uint8_t j = 0; j < kNumPlaneSetNormals; ++j) {
				if (m_scene_objects[i]->GetRTIntersectionMethod() != RT_INTR_TYPE::sphere)
				{
					for (const auto ind : m_scene_objects[i]->m_mesh.m_indices) {
						float d = glm::dot(planeSetNormals[j], m_scene_objects[i]->m_mesh.m_vertex_positions[ind]);
						// set dNear and dFar
						if (d < extentsList[i].d[j][0]) extentsList[i].d[j][0] = d - 0.00001;//make bbox slightly larger incase of a thin mesh
						if (d > extentsList[i].d[j][1]) extentsList[i].d[j][1] = d + 0.00001;
					}

				}
				else//Sphere case
				{
					float step_a = glm::pi<float>() / 18.0f;
					float step_b = 0.001f;
					for(float theta = 0;  theta < 2 * glm::pi<float>(); theta+= step_a)//Sample points over the sphere's surface
					{
						for (float k = 0; k < 1.0f; k+=step_b)
						{
							float phi = acos(1 - 2 * k);
							glm::vec3 point = { sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi) };
							point = point * m_scene_objects[i]->m_radius + m_scene_objects[i]->GetPosition();

							float d = glm::dot(planeSetNormals[j], point);
							// set dNear and dFar
							if (d < extentsList[i].d[j][0]) extentsList[i].d[j][0] = d;
							if (d > extentsList[i].d[j][1]) extentsList[i].d[j][1] = d;
						}
					}
				}
			}
			sceneExtents.extendBy(extentsList[i]); // expand the scene extent of this object's extent 
			extentsList[i].obj = m_scene_objects[i].get(); // the extent itself needs to keep a pointer to the object its holds 
		}

		// Now that we have the extent of the scene we can start building our octree
		octree = new Octree(sceneExtents);

		for (uint32_t i = 0; i < m_scene_objects.size(); ++i) {
			octree->Insert(&extentsList[i]);
		}

		// Build from bottom up
		octree->Build();
	}

	bool BVH::Extents::Intersect(
		const float* precomputedNumerator,
		const float* precomputedDenominator,
		float& tNear,   // tn and tf in this method need to be contained 
		float& tFar,    // within the range [tNear:tFar] 
		uint8_t& planeIndex) const
	{
		for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
			float tNearExtents = (d[i][0] - precomputedNumerator[i]) / precomputedDenominator[i];
			float tFarExtents = (d[i][1] - precomputedNumerator[i]) / precomputedDenominator[i];
			if (precomputedDenominator[i] < 0) std::swap(tNearExtents, tFarExtents);
			if (tNearExtents > tNear) tNear = tNearExtents, planeIndex = i;
			if (tFarExtents < tFar) tFar = tFarExtents;
			if (tNear > tFar) return false;
		}

		return true;
	}

	bool BVH::Intersect(Ray ray, float intersect_eps, IntersectionData* isec_data) const
	{
		float tHit = std::numeric_limits<float>().max();
		const SceneObject* intersected_obj = nullptr;
		float precomputedNumerator[BVH::kNumPlaneSetNormals];
		float precomputedDenominator[BVH::kNumPlaneSetNormals];
		for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
			precomputedNumerator[i] = dot(planeSetNormals[i], ray.origin);
			precomputedDenominator[i] = dot(planeSetNormals[i], ray.direction);
		}

		
		/*float tNear = std::numeric_limits<float>().max(); // set
		for (uint32_t i = 0; i < m_scene_objects.size(); ++i) {
			//numRayVolumeTests++;
			float tn = -std::numeric_limits<float>().max(), tf = std::numeric_limits<float>().max();
			uint8_t planeIndex;
			if (extentsList[i].Intersect(precomputedNumerator, precomputedDenominator, tn, tf, planeIndex)) {
				if (tn < tNear) {
					intersected_obj = m_scene_objects[i].get();
					tNear = tn;
					// normal = planeSetNormals[planeIndex];
				}
			}
		}*/
		

		uint8_t planeIndex;
		float tNear = 0, tFar = std::numeric_limits<float>().max(); // tNear, tFar for the intersected extents 
		if (!octree->root->nodeExtents.Intersect(precomputedNumerator, precomputedDenominator, tNear, tFar, planeIndex) || tFar < 0)
			return false;
		tHit = tFar;
		std::priority_queue<BVH::Octree::QueueElement> queue;
		queue.push(BVH::Octree::QueueElement(octree->root, 0));
		while (!queue.empty() && queue.top().t < tHit) 
		{
			IntersectionData* tmp_data = new IntersectionData();

			const Octree::OctreeNode* node = queue.top().node;
			queue.pop();
			if (node->isLeaf) {
				for (const auto& e : node->nodeExtentsList) {
					float t = std::numeric_limits<float>().max();
					//CH_TRACE(std::to_string((e->obj->Intersect(ray, 0.000001f, tmp_data))) + ", " + std::to_string((glm::distance(tmp_data->position, ray.origin) < (glm::distance(isec_data->position, ray.origin)))));
					//CH_TRACE(std::to_string(glm::distance(isec_data->position, ray.origin)));
					if ( e->obj->Intersect(ray, intersect_eps, tmp_data) && tmp_data->t < tHit)  {
						*isec_data = *tmp_data;
						tHit = tmp_data->t;
						intersected_obj = e->obj;
						//CH_TRACE(std::to_string((e->obj->Intersect(ray, 0.000001f, tmp_data))) + ", " + e->obj->GetName());
					}
				}
			}
			else {
				for (uint8_t i = 0; i < 8; ++i) {
					if (node->child[i] != nullptr) {
						float tNearChild = 0, tFarChild = tFar;
						if (node->child[i]->nodeExtents.Intersect(precomputedNumerator, precomputedDenominator, tNearChild, tFarChild, planeIndex)) {
							float t = (tNearChild < 0 && tFarChild >= 0) ? tFarChild : tNearChild;
							queue.push(BVH::Octree::QueueElement(node->child[i], t));
							//CH_TRACE(" t: " + std::to_string(t));
						}
					}
				}
			}
			delete tmp_data;
		}

		return (intersected_obj != nullptr);
	}
}