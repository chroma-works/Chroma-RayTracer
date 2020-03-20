#include "BVH.h"

#include <algorithm>
#include <ctime>
#include <iostream>



#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/gtx/euler_angles.hpp>
#include <thirdparty/glm/glm/gtx/intersect.hpp>
#include <thirdparty/glm/glm/gtx/norm.hpp>
#include <thirdparty/glm/glm/gtc/matrix_transform.hpp>
#include <thirdparty/glm/glm/gtc/type_ptr.hpp>

#include <ray-tracer/main/Scene.h>

//src:https://github.com/mmp/pbrt-v3/blob/master/src/accelerators/bvh.cpp

namespace Chroma
{
	struct BVHPrimitiveInfo {
		BVHPrimitiveInfo() {}
		BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3& bounds)
			: primitiveNumber(primitiveNumber),
			bounds(bounds),
			centroid(.5f * bounds.min + .5f * bounds.max) {}
		size_t primitiveNumber;
		Bounds3 bounds;
		glm::vec3 centroid;
	};

	struct BVHBuildNode {
		// BVHBuildNode Public Methods
		void InitLeaf(int first, int n, const Bounds3& b) {
			first_prim_offset = first;
			n_primitives = n;
			bounds = b;
			children[0] = children[1] = nullptr;
		}
		void InitInterior(int axis, BVHBuildNode* c0, BVHBuildNode* c1) {
			children[0] = c0;
			children[1] = c1;
			bounds = Bounds3::Extend(c0->bounds, c1->bounds);
			split_axis = axis;
			n_primitives = 0;
		}
		Bounds3 bounds;
		BVHBuildNode* children[2];
		int split_axis, first_prim_offset, n_primitives;
	};

	struct MortonPrimitive {
		int primitiveIndex;
		uint32_t mortonCode;
	};

	struct LBVHTreelet {
		int start_index, n_primitives;
		BVHBuildNode* build_nodes;
	};

	struct LinearBVHNode {
		Bounds3 bounds;
		union {
			int primitives_offset;   // leaf
			int second_child_offset;  // interior
		};
		uint16_t nPrimitives;  // 0 -> interior node
		uint8_t axis;          // interior node: xyz
		uint8_t pad[1];        // ensure 32 byte total size
	};

	// Bvh Utility Functions
	inline uint32_t LeftShift3(uint32_t x) {
		//CHECK_LE(x, (1 << 10));
		if (x == (1 << 10)) --x;
#ifdef PBRT_HAVE_BINARY_CONSTANTS
		x = (x | (x << 16)) & 0b00000011000000000000000011111111;
		// x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x | (x << 8)) & 0b00000011000000001111000000001111;
		// x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x | (x << 4)) & 0b00000011000011000011000011000011;
		// x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x | (x << 2)) & 0b00001001001001001001001001001001;
		// x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
#else
		x = (x | (x << 16)) & 0x30000ff;
		// x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x | (x << 8)) & 0x300f00f;
		// x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x | (x << 4)) & 0x30c30c3;
		// x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x | (x << 2)) & 0x9249249;
		// x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
#endif // PBRT_HAVE_BINARY_CONSTANTS
		return x;
	}

	inline uint32_t EncodeMorton3(const glm::vec3& v) {
		//CHECK_GE(v.x, 0);
		//CHECK_GE(v.y, 0);
		//CHECK_GE(v.z, 0);
		return (LeftShift3(v.z) << 2) | (LeftShift3(v.y) << 1) | LeftShift3(v.x);
	}

	static void RadixSort(std::vector<MortonPrimitive>* v) {
		std::vector<MortonPrimitive> tempVector(v->size());
		constexpr int bitsPerPass = 6;
		constexpr int nBits = 30;
		static_assert((nBits % bitsPerPass) == 0,
			"Radix sort bitsPerPass must evenly divide nBits");
		constexpr int nPasses = nBits / bitsPerPass;

		for (int pass = 0; pass < nPasses; ++pass) {
			// Perform one pass of radix sort, sorting _bitsPerPass_ bits
			int lowBit = pass * bitsPerPass;

			// Set in and out vector pointers for radix sort pass
			std::vector<MortonPrimitive>& in = (pass & 1) ? tempVector : *v;
			std::vector<MortonPrimitive>& out = (pass & 1) ? *v : tempVector;

			// Count number of zero bits in array for current radix sort bit
			constexpr int nBuckets = 1 << bitsPerPass;
			int bucketCount[nBuckets] = { 0 };
			constexpr int bitMask = (1 << bitsPerPass) - 1;
			for (const MortonPrimitive& mp : in) {
				int bucket = (mp.mortonCode >> lowBit)& bitMask;
				//CHECK_GE(bucket, 0);
				//CHECK_LT(bucket, nBuckets);
				++bucketCount[bucket];
			}

			// Compute starting index in output array for each bucket
			int outIndex[nBuckets];
			outIndex[0] = 0;
			for (int i = 1; i < nBuckets; ++i)
				outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];

			// Store sorted values in output array
			for (const MortonPrimitive& mp : in) {
				int bucket = (mp.mortonCode >> lowBit)& bitMask;
				out[outIndex[bucket]++] = mp;
			}
		}
		// Copy final result from _tempVector_, if needed
		if (nPasses & 1) std::swap(*v, tempVector);
	}


	BVH::BVH(Scene& scene,
		int maxPrimsInNode, SplitMethod splitMethod)
		: m_max_prims_in_node(std::min(255, maxPrimsInNode)),
		m_split_method(splitMethod) {

		clock_t start_time = clock();
		//ProfilePhase _(Prof::AccelConstruction);

		// Build BVH from _primitives_

		// Initialize _primitiveInfo_ array for primitives
		m_scene_ptr = &scene;

		InitShapes();

		std::vector<BVHPrimitiveInfo> primitiveInfo(m_shapes.size());

		for (size_t i = 0; i < m_shapes.size(); ++i)
		{
			glm::vec3 b_min, b_max;

			/*switch (m_shapes[i].m_type)
			{
			case SHAPE_T::sphere:
			{
				const Sphere* shape = (Sphere*)&m_shapes[i];
				b_min = ((Sphere*)(&shape))->m_center -
					glm::vec3(((Sphere*)(&shape))->m_radius,
					((Sphere*)(&shape))->m_radius,
						((Sphere*)(&shape))->m_radius);

				b_max = ((Sphere*)(&shape))->m_center +
					glm::vec3(((Sphere*)(&shape))->m_radius,
					((Sphere*)(&shape))->m_radius,
						((Sphere*)(&shape))->m_radius);
			}
				break;
			case SHAPE_T::triangle:
			{
				const Triangle* shape = (Triangle*)&m_shapes[i];
				b_min = ((Triangle*)shape)->m_vertices[0];
				b_max = ((Triangle*)shape)->m_vertices[0];

				b_min = glm::min(b_min, ((Triangle*)shape)->m_vertices[1]);
				b_min = glm::min(b_min, ((Triangle*)shape)->m_vertices[2]);

				b_max = glm::max(b_max, ((Triangle*)shape)->m_vertices[1]);
				b_max = glm::max(b_max, ((Triangle*)shape)->m_vertices[2]);
			}
				break;
			}*/
			
			Bounds3 b = m_shapes[i]->GetBounds();
			primitiveInfo[i] = { i, b};
		}

		// Build BVH tree for primitives using _primitiveInfo_
		MemoryArena arena(1024 * 1024);
		int totalNodes = 0;
		std::vector<Shape*> orderedPrims;
		orderedPrims.reserve(m_shapes.size());
		BVHBuildNode* root;
		if (splitMethod == SplitMethod::HLBVH)
			root = HLBVHBuild(arena, primitiveInfo, &totalNodes, orderedPrims);
		else
			root = RecursiveBuild(arena, primitiveInfo, 0, m_shapes.size(),
				&totalNodes, orderedPrims);
		m_shapes.swap(orderedPrims);
		/*LOG(INFO) << StringPrintf("BVH created with %d nodes for %d "
			"primitives (%.2f MB)", totalNodes,
			(int)primitives.size(),
			float(totalNodes * sizeof(LinearBVHNode)) /
			(1024.f * 1024.f));*/

			//// Compute representation of depth-first traversal of BVH tree
			//treeBytes += totalNodes * sizeof(LinearBVHNode) + sizeof(*this) +
			//    primitives.size() * sizeof(primitives[0]);
		m_nodes = AllocAligned<LinearBVHNode>(totalNodes);
		int offset = 0;
		FlattenBVHTree(root, &offset);

		CH_TRACE("BVH info:\n\tNode count: " + 
			std::to_string(totalNodes) +
			"\n\tNode size:  " + std::to_string(sizeof(LinearBVHNode)) +
			"\n\tBVH size:  " +  
			std::to_string((totalNodes * sizeof(LinearBVHNode)) 
			/ (1024.0f * 1024.0f)) + "MB");

		clock_t elapsed = clock() - start_time;
	}

	Bounds3 BVH::WorldBound() const {
		return m_nodes ? m_nodes[0].bounds : Bounds3();
	}

	struct BucketInfo {
		int count = 0;
		Bounds3 bounds;
	};

	BVHBuildNode* BVH::RecursiveBuild(
		MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfo, int start,
		int end, int* totalNodes,
		std::vector<Shape*>& orderedPrims) {
		//CHECK_NE(start, end);
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		(*totalNodes)++;
		// Compute bounds of all primitives in BVH node
		Bounds3 bounds;
		for (int i = start; i < end; ++i)
			bounds.Extend(primitiveInfo[i].bounds);
		int nPrimitives = end - start;
		if (nPrimitives == 1) {
			// Create leaf _BVHBuildNode_
			int firstPrimOffset = orderedPrims.size();
			for (int i = start; i < end; ++i) {
				int primNum = primitiveInfo[i].primitiveNumber;
				orderedPrims.push_back(m_shapes[primNum]);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else {
			// Compute bound of primitive centroids, choose split dimension _dim_
			Bounds3 centroidBounds;
			for (int i = start; i < end; ++i)
				centroidBounds.Extend(primitiveInfo[i].centroid);
			int dim = centroidBounds.MaxExtent();

			// Partition primitives into two sets and build children
			int mid = (start + end) / 2;
			if (centroidBounds.max[dim] == centroidBounds.min[dim]) {
				// Create leaf _BVHBuildNode_
				int firstPrimOffset = orderedPrims.size();
				for (int i = start; i < end; ++i) {
					int primNum = primitiveInfo[i].primitiveNumber;
					orderedPrims.push_back(m_shapes[primNum]);
				}
				node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
				return node;
			}
			else {
				// Partition primitives based on _splitMethod_
				switch (m_split_method) {
				case SplitMethod::Middle: {
					// Partition primitives through node's midpoint
					float pmid =
						(centroidBounds.min[dim] + centroidBounds.max[dim]) / 2;
					BVHPrimitiveInfo* midPtr = std::partition(
						&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
						[dim, pmid](const BVHPrimitiveInfo& pi) {
						return pi.centroid[dim] < pmid;
					});
					mid = midPtr - &primitiveInfo[0];
					// For lots of prims with large overlapping bounding boxes, this
					// may fail to partition; in that case don't break and fall
					// through
					// to EqualCounts.
					if (mid != start && mid != end) break;
				}
				case SplitMethod::EqualCounts: {
					// Partition primitives into equally-sized subsets
					mid = (start + end) / 2;
					std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
						&primitiveInfo[end - 1] + 1,
						[dim](const BVHPrimitiveInfo& a,
							const BVHPrimitiveInfo& b) {
						return a.centroid[dim] < b.centroid[dim];
					});
					break;
				}
				case SplitMethod::SAH:
				default: {
					// Partition primitives using approximate SAH
					if (nPrimitives <= 2) {
						// Partition primitives into equally-sized subsets
						mid = (start + end) / 2;
						std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
							&primitiveInfo[end - 1] + 1,
							[dim](const BVHPrimitiveInfo& a,
								const BVHPrimitiveInfo& b) {
							return a.centroid[dim] <
								b.centroid[dim];
						});
					}
					else {
						// Allocate _BucketInfo_ for SAH partition buckets
						constexpr int nBuckets = 12;
						BucketInfo buckets[nBuckets];

						// Initialize _BucketInfo_ for SAH partition buckets
						for (int i = start; i < end; ++i) {
							int b = nBuckets *
								centroidBounds.Offset(
									primitiveInfo[i].centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							//CHECK_GE(b, 0);
							//CHECK_LT(b, nBuckets);
							buckets[b].count++;
							buckets[b].bounds.Extend(primitiveInfo[i].bounds);
						}

						// Compute costs for splitting after each bucket
						float cost[nBuckets - 1];
						for (int i = 0; i < nBuckets - 1; ++i) {
							Bounds3 b0, b1;
							int count0 = 0, count1 = 0;
							for (int j = 0; j <= i; ++j) {
								b0.Extend(buckets[j].bounds);
								count0 += buckets[j].count;
							}
							for (int j = i + 1; j < nBuckets; ++j) {
								b1.Extend(buckets[j].bounds);
								count1 += buckets[j].count;
							}
							cost[i] = 1 +
								(count0 * b0.GetSurfaceArea() +
									count1 * b1.GetSurfaceArea()) /
								bounds.GetSurfaceArea();
						}

						// Find bucket to split at that minimizes SAH metric
						float minCost = cost[0];
						int minCostSplitBucket = 0;
						for (int i = 1; i < nBuckets - 1; ++i) {
							if (cost[i] < minCost) {
								minCost = cost[i];
								minCostSplitBucket = i;
							}
						}

						// Either create leaf or split primitives at selected SAH
						// bucket
						float leafCost = nPrimitives;
						if (nPrimitives > m_max_prims_in_node || minCost < leafCost) {
							BVHPrimitiveInfo* pmid = std::partition(
								&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
								[=](const BVHPrimitiveInfo& pi) {
								int b = nBuckets *
									centroidBounds.Offset(pi.centroid)[dim];
								if (b == nBuckets) b = nBuckets - 1;
								//CHECK_GE(b, 0);
								//CHECK_LT(b, nBuckets);
								return b <= minCostSplitBucket;
							});
							mid = pmid - &primitiveInfo[0];
						}
						else {
							// Create leaf _BVHBuildNode_
							int firstPrimOffset = orderedPrims.size();
							for (int i = start; i < end; ++i) {
								int primNum = primitiveInfo[i].primitiveNumber;
								orderedPrims.push_back(m_shapes[primNum]);
							}
							node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
							return node;
						}
					}
					break;
				}
				}
				node->InitInterior(dim,
					RecursiveBuild(arena, primitiveInfo, start, mid,
						totalNodes, orderedPrims),
					RecursiveBuild(arena, primitiveInfo, mid, end,
						totalNodes, orderedPrims));
			}
		}
		return node;
	}

	BVHBuildNode* BVH::HLBVHBuild(
		MemoryArena& arena, const std::vector<BVHPrimitiveInfo>& primitiveInfo,
		int* totalNodes,
		std::vector<Shape*>& orderedPrims) const {
		// Compute bounding box of all primitive centroids
		Bounds3 bounds;
		for (const BVHPrimitiveInfo& pi : primitiveInfo)
			bounds.Extend(pi.centroid);

		// Compute Morton indices of primitives
		std::vector<MortonPrimitive> mortonPrims(primitiveInfo.size());
		for (int i = 0; i < primitiveInfo.size(); i++) {
			// Initialize _mortonPrims[i]_ for _i_th primitive
			constexpr int mortonBits = 10;
			constexpr int mortonScale = 1 << mortonBits;
			mortonPrims[i].primitiveIndex = primitiveInfo[i].primitiveNumber;
			glm::vec3 centroidOffset = bounds.Offset(primitiveInfo[i].centroid);
			mortonPrims[i].mortonCode = EncodeMorton3(centroidOffset * (float)mortonScale);
		}//, primitiveInfo.size(), 512);

		// Radix sort primitive Morton indices
		RadixSort(&mortonPrims);

		// Create LBVH treelets at bottom of BVH

		// Find intervals of primitives for each treelet
		std::vector<LBVHTreelet> treeletsToBuild;
		for (int start = 0, end = 1; end <= (int)mortonPrims.size(); ++end) {
#ifdef PBRT_HAVE_BINARY_CONSTANTS
			uint32_t mask = 0b00111111111111000000000000000000;
#else
			uint32_t mask = 0x3ffc0000;
#endif
			if (end == (int)mortonPrims.size() ||
				((mortonPrims[start].mortonCode & mask) !=
				(mortonPrims[end].mortonCode & mask))) {
				// Add entry to _treeletsToBuild_ for this treelet
				int nPrimitives = end - start;
				int maxBVHNodes = 2 * nPrimitives;
				BVHBuildNode* nodes = arena.Alloc<BVHBuildNode>(maxBVHNodes, false);
				treeletsToBuild.push_back({ start, nPrimitives, nodes });
				start = end;
			}
		}

		// Create LBVHs for treelets in parallel
		std::atomic<int> atomicTotal(0), orderedPrimsOffset(0);
		orderedPrims.resize(m_shapes.size());
		for (int i = 0; i < treeletsToBuild.size(); i++) {
			// Generate _i_th LBVH treelet
			int nodesCreated = 0;
			const int firstBitIndex = 29 - 12;
			LBVHTreelet& tr = treeletsToBuild[i];
			tr.build_nodes =
				EmitLBVH(tr.build_nodes, primitiveInfo, &mortonPrims[tr.start_index],
					tr.n_primitives, &nodesCreated, orderedPrims,
					&orderedPrimsOffset, firstBitIndex);
			atomicTotal += nodesCreated;
		}//, treeletsToBuild.size());
		*totalNodes = atomicTotal;

		// Create and return SAH BVH from LBVH treelets
		std::vector<BVHBuildNode*> finishedTreelets;
		finishedTreelets.reserve(treeletsToBuild.size());
		for (LBVHTreelet& treelet : treeletsToBuild)
			finishedTreelets.push_back(treelet.build_nodes);
		return BuildUpperSAH(arena, finishedTreelets, 0, finishedTreelets.size(),
			totalNodes);
	}

	BVHBuildNode* BVH::EmitLBVH(
		BVHBuildNode*& buildNodes,
		const std::vector<BVHPrimitiveInfo>& primitiveInfo,
		MortonPrimitive* mortonPrims, int nPrimitives, int* totalNodes,
		std::vector<Shape*>& orderedPrims,
		std::atomic<int>* orderedPrimsOffset, int bitIndex) const {
		//CHECK_GT(nPrimitives, 0);
		if (bitIndex == -1 || nPrimitives < m_max_prims_in_node) {
			// Create and return leaf node of LBVH treelet
			(*totalNodes)++;
			BVHBuildNode* node = buildNodes++;
			Bounds3 bounds;
			int firstPrimOffset = orderedPrimsOffset->fetch_add(nPrimitives);
			for (int i = 0; i < nPrimitives; ++i) {
				int primitiveIndex = mortonPrims[i].primitiveIndex;
				orderedPrims[firstPrimOffset + i] = m_shapes[primitiveIndex];
				bounds.Extend(primitiveInfo[primitiveIndex].bounds);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else {
			int mask = 1 << bitIndex;
			// Advance to next subtree level if there's no LBVH split for this bit
			if ((mortonPrims[0].mortonCode & mask) ==
				(mortonPrims[nPrimitives - 1].mortonCode & mask))
				return EmitLBVH(buildNodes, primitiveInfo, mortonPrims, nPrimitives,
					totalNodes, orderedPrims, orderedPrimsOffset,
					bitIndex - 1);

			// Find LBVH split point for this dimension
			int searchStart = 0, searchEnd = nPrimitives - 1;
			while (searchStart + 1 != searchEnd) {
				//CHECK_NE(searchStart, searchEnd);
				int mid = (searchStart + searchEnd) / 2;
				if ((mortonPrims[searchStart].mortonCode & mask) ==
					(mortonPrims[mid].mortonCode & mask))
					searchStart = mid;
				else {
					//CHECK_EQ(mortonPrims[mid].mortonCode & mask,
					   // mortonPrims[searchEnd].mortonCode & mask);
					searchEnd = mid;
				}
			}
			int splitOffset = searchEnd;
			//CHECK_LE(splitOffset, nPrimitives - 1);
			//CHECK_NE(mortonPrims[splitOffset - 1].mortonCode & mask,
			//    mortonPrims[splitOffset].mortonCode & mask);

			// Create and return interior LBVH node
			(*totalNodes)++;
			BVHBuildNode* node = buildNodes++;
			BVHBuildNode* lbvh[2] = {
				EmitLBVH(buildNodes, primitiveInfo, mortonPrims, splitOffset,
				totalNodes, orderedPrims, orderedPrimsOffset,
				bitIndex - 1),
				EmitLBVH(buildNodes, primitiveInfo, &mortonPrims[splitOffset],
				nPrimitives - splitOffset, totalNodes, orderedPrims,
				orderedPrimsOffset, bitIndex - 1) };
			int axis = bitIndex % 3;
			node->InitInterior(axis, lbvh[0], lbvh[1]);
			return node;
		}
	}

	BVHBuildNode* BVH::BuildUpperSAH(MemoryArena& arena,
		std::vector<BVHBuildNode*>& treeletRoots,
		int start, int end,
		int* totalNodes) const {
		//CHECK_LT(start, end);
		int nNodes = end - start;
		if (nNodes == 1) return treeletRoots[start];
		(*totalNodes)++;
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();

		// Compute bounds of all nodes under this HLBVH node
		Bounds3 bounds;
		for (int i = start; i < end; ++i)
			bounds.Extend(treeletRoots[i]->bounds);

		// Compute bound of HLBVH node centroids, choose split dimension _dim_
		Bounds3 centroidBounds;
		for (int i = start; i < end; ++i) {
			glm::vec3 centroid =
				(treeletRoots[i]->bounds.min + treeletRoots[i]->bounds.max) *
				0.5f;
			centroidBounds.Extend(centroid);
		}
		int dim = centroidBounds.MaxExtent();
		// FIXME: if this hits, what do we need to do?
		// Make sure the SAH split below does something... ?
		//CHECK_NE(centroidBounds.max[dim], centroidBounds.min[dim]);

		// Allocate _BucketInfo_ for SAH partition buckets
		constexpr int nBuckets = 12;
		struct BucketInfo {
			int count = 0;
			Bounds3 bounds;
		};
		BucketInfo buckets[nBuckets];

		// Initialize _BucketInfo_ for HLBVH SAH partition buckets
		for (int i = start; i < end; ++i) {
			float centroid = (treeletRoots[i]->bounds.min[dim] +
				treeletRoots[i]->bounds.max[dim]) *
				0.5f;
			int b =
				nBuckets * ((centroid - centroidBounds.min[dim]) /
				(centroidBounds.max[dim] - centroidBounds.min[dim]));
			if (b == nBuckets) b = nBuckets - 1;
			/*CHECK_GE(b, 0);
			CHECK_LT(b, nBuckets);*/
			buckets[b].count++;
			buckets[b].bounds.Extend(treeletRoots[i]->bounds);
		}

		// Compute costs for splitting after each bucket
		float cost[nBuckets - 1];
		for (int i = 0; i < nBuckets - 1; ++i) {
			Bounds3 b0, b1;
			int count0 = 0, count1 = 0;
			for (int j = 0; j <= i; ++j) {
				b0.Extend(buckets[j].bounds);
				count0 += buckets[j].count;
			}
			for (int j = i + 1; j < nBuckets; ++j) {
				b1.Extend(buckets[j].bounds);
				count1 += buckets[j].count;
			}
			cost[i] = .125f +
				(count0 * b0.GetSurfaceArea() + count1 * b1.GetSurfaceArea()) /
				bounds.GetSurfaceArea();
		}

		// Find bucket to split at that minimizes SAH metric
		float minCost = cost[0];
		int minCostSplitBucket = 0;
		for (int i = 1; i < nBuckets - 1; ++i) {
			if (cost[i] < minCost) {
				minCost = cost[i];
				minCostSplitBucket = i;
			}
		}

		// Split nodes and create interior HLBVH SAH node
		BVHBuildNode** pmid = std::partition(
			&treeletRoots[start], &treeletRoots[end - 1] + 1,
			[=](const BVHBuildNode* node) {
			float centroid =
				(node->bounds.min[dim] + node->bounds.max[dim]) * 0.5f;
			int b = nBuckets *
				((centroid - centroidBounds.min[dim]) /
				(centroidBounds.max[dim] - centroidBounds.min[dim]));
			if (b == nBuckets) b = nBuckets - 1;
			//CHECK_GE(b, 0);
			//CHECK_LT(b, nBuckets);
			return b <= minCostSplitBucket;
		});
		int mid = pmid - &treeletRoots[0];
		//CHECK_GT(mid, start);
		//CHECK_LT(mid, end);
		node->InitInterior(
			dim, this->BuildUpperSAH(arena, treeletRoots, start, mid, totalNodes),
			this->BuildUpperSAH(arena, treeletRoots, mid, end, totalNodes));
		return node;
	}

	int BVH::FlattenBVHTree(BVHBuildNode* node, int* offset) {
		LinearBVHNode* linearNode = &m_nodes[*offset];
		linearNode->bounds = node->bounds;
		int myOffset = (*offset)++;
		if (node->n_primitives > 0) {
			//CHECK(!node->children[0] && !node->children[1]);
			//CHECK_LT(node->nPrimitives, 65536);
			linearNode->primitives_offset = node->first_prim_offset;
			linearNode->nPrimitives = node->n_primitives;
		}
		else {
			// Create interior flattened BVH node
			linearNode->axis = node->split_axis;
			linearNode->nPrimitives = 0;
			FlattenBVHTree(node->children[0], offset);
			linearNode->second_child_offset =
				FlattenBVHTree(node->children[1], offset);
		}
		return myOffset;
	}

	BVH::~BVH()
	{
		FreeAligned(m_nodes);

		//for (Face face : faces)
		//{
		//	face.reset();
		//}
	}

	bool BVH::Intersect(const Ray& ray, IntersectionData* intersection_data) const {
		if (!m_nodes) return false;
		//ProfilePhase p(Prof::AccelIntersect);
		glm::vec3 invDir = glm::vec3(1.0f/ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);
		int dirIsNeg[3] = { invDir.x < ray.intersect_eps, invDir.y < ray.intersect_eps, invDir.z < ray.intersect_eps };
		// Follow ray through BVH nodes to find primitive intersections
		int toVisitOffset = 0, currentNodeIndex = 0;
		int nodesToVisit[64];
		float t_min = std::numeric_limits<float>().max();
		IntersectionData probe_data;
		while (true) 
		{
			const LinearBVHNode* node = &m_nodes[currentNodeIndex];
			// Check ray against BVH node
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg)) {
				if (node->nPrimitives > 0) {
					// Intersect ray with primitives in leaf BVH node
					for (int i = 0; i < node->nPrimitives; ++i)
					{
						const Shape* s =
							m_shapes[node->primitives_offset + i];
						// Check one primitive inside leaf node
						probe_data.t = INFINITY;
						if (s->Intersect(ray, &probe_data) && s->m_is_visible)
						{
							if (probe_data.t < intersection_data->t /*&& probe_data->t >ray.intersect_eps*/)
							{
								*intersection_data = probe_data;
							}

							//intersection_data->hit = true;
						}
					}
					if (toVisitOffset == 0) break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else {
					// Put far BVH node on _nodesToVisit_ stack, advance to near
					// node
					if (dirIsNeg[node->axis]) {
						nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
						currentNodeIndex = node->second_child_offset;
					}
					else {
						nodesToVisit[toVisitOffset++] = node->second_child_offset;
						currentNodeIndex = currentNodeIndex + 1;
					}
				}
			}
			else {
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
		}
		return intersection_data->hit;
	}

	bool BVH::IntersectP(const Ray& ray) const
	{
		return false;
	}

	int BVH::GetSizeBytes()
	{
		return 0;
	}

	void BVH::InitShapes()
	{
		Scene& scene = *m_scene_ptr;

		std::vector<SceneObject*> scene_objects;

		for (auto obj : scene.m_scene_objects)
		{
			scene_objects.push_back(obj.second.get());
		}

		for (int i = 0, face_idx = 0; i < scene_objects.size(); i++)
		{
			if (scene_objects[i]->GetShapeType() != SHAPE_T::sphere)
			{
				auto mesh = scene_objects[i]->m_mesh;

				if (mesh == nullptr)
					continue;

				glm::mat4 t = glm::translate(glm::mat4(1.0f), scene_objects[i]->GetPosition());
				glm::vec3 rot = glm::radians(scene_objects[i]->GetRotation());
				glm::mat4 r = glm::eulerAngleYXZ(rot.y, rot.x, rot.z);
				glm::mat4 s = glm::scale(glm::mat4(1.0), scene_objects[i]->GetScale());

				s[3][3] = 1;

				glm::mat4 m = t * r * s;

				for (int j = 0; j < mesh->m_indices.size(); j += 3)
				{

					Triangle* tri = new Triangle(scene_objects[i]->GetMaterial(), scene_objects[i]->IsVisible());

					tri->m_vertices[0] = glm::vec3(m *
						glm::vec4(mesh->m_vertex_positions[mesh->m_indices[j]], 1));
					tri->m_vertices[1] = glm::vec3(m *
						glm::vec4(mesh->m_vertex_positions[mesh->m_indices[j + 1]], 1));
					tri->m_vertices[2] = glm::vec3(m *
						glm::vec4(mesh->m_vertex_positions[mesh->m_indices[j + 2]], 1));

					tri->m_normals[0] = glm::normalize(glm::vec3(r *
						glm::vec4(mesh->m_vertex_normals[mesh->m_indices[j]], 1)));
					tri->m_normals[1] = glm::normalize(glm::vec3(r *
						glm::vec4(mesh->m_vertex_normals[mesh->m_indices[j + 1]], 1)));
					tri->m_normals[2] = glm::normalize(glm::vec3(r *
						glm::vec4(mesh->m_vertex_normals[mesh->m_indices[j + 2]], 1)));

					//if (mesh->uvs.size() > 0)
					//{
					//    shape.uvs[0] = mesh->uvs[j + 0];
					//    shape.uvs[1] = mesh->uvs[j + 1];
					//    shape.uvs[2] = mesh->uvs[j + 2];
					//}

					m_shapes.push_back(tri);
				}
			}
			else//Sphere
			{
				Sphere* sphere = new Sphere(scene_objects[i]->GetMaterial(), scene_objects[i]->IsVisible());
				sphere->m_center = scene_objects[i]->GetPosition();
				sphere->m_radius = scene_objects[i]->m_radius;

				m_shapes.push_back(sphere);
			}
		}
	}
}