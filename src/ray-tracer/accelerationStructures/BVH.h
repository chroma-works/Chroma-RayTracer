#pragma once

#include <atomic>

#include <ray-tracer/accelerationStructures/AccelerationStructure.h>
#include <ray-tracer/main/Shape.h>
#include <ray-tracer/main/Geometry.h>
#include <ray-tracer/accelerationStructures/Memory.h>

#include <memory>
#include <vector>
//src:https://github.com/mmp/pbrt-v3/blob/master/src/accelerators/bvh.h


namespace Chroma
{
	class Scene;

	struct BVHBuildNode;

	// Bvh Forward Declarations
	struct BVHPrimitiveInfo;
	struct MortonPrimitive;
	struct LinearBVHNode;

	enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
	// Bvh Declarations
	class BVH : public AccelerationStructure
	{
	public:
		// Bvh Public Methods
		BVH(Scene& scene,
			int maxPrimsInNode = 1,
			SplitMethod splitMethod = SplitMethod::SAH);
		Bounds3 WorldBound() const;
		~BVH();
		bool Intersect(const Ray& ray, IntersectionData* intersection_data) const;
		bool IntersectP(const Ray& ray) const;

		int GetSizeBytes();

		void InitShapes();
		// Bvh Private Methods
		BVHBuildNode* RecursiveBuild(
			MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfo,
			int start, int end, int* totalNodes,
			std::vector<Shape*>& orderedPrims);
		BVHBuildNode* HLBVHBuild(
			MemoryArena& arena, const std::vector<BVHPrimitiveInfo>& primitiveInfo,
			int* totalNodes,
			std::vector<Shape*>& orderedPrims) const;
		BVHBuildNode* EmitLBVH(
			BVHBuildNode*& buildNodes,
			const std::vector<BVHPrimitiveInfo>& primitiveInfo,
			MortonPrimitive* mortonPrims, int nPrimitives, int* totalNodes,
			std::vector<Shape*>& orderedPrims,
			std::atomic<int>* orderedPrimsOffset, int bitIndex) const;
		BVHBuildNode* BuildUpperSAH(MemoryArena& arena,
			std::vector<BVHBuildNode*>& treeletRoots,
			int start, int end, int* totalNodes) const;
		int FlattenBVHTree(BVHBuildNode* node, int* offset);

		std::vector<Bounds3> m_prim_bounds, m_leaf_bounds;
		Scene* m_scene_ptr;
		std::vector<Shape*> m_shapes;

		// Bvh Private Data
		const int m_max_prims_in_node;
		const SplitMethod m_split_method;
		//std::vector<Face> faces;
		LinearBVHNode* m_nodes = nullptr;
	};
}