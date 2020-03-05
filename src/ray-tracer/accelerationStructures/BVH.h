#pragma once
#include <ray-tracer/accelerationStructures/AccelerationStructure.h>
//source: https://www.scratchapixel.com/code.php?id=36&origin=/lessons/advanced-rendering/introduction-acceleration-structure



namespace Chroma
{
	class BVH : public AccelerationStructure
	{
		static const uint8_t kNumPlaneSetNormals = 3;
		static const glm::vec3 planeSetNormals[kNumPlaneSetNormals];
		struct Extents
		{
			Extents()
			{
				for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i)
					d[i][0] = std::numeric_limits<float>().max(), d[i][1] = -std::numeric_limits<float>().max();
			}
			void extendBy(const Extents& e)
			{

				for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
					if (e.d[i][0] < d[i][0]) d[i][0] = e.d[i][0];
					if (e.d[i][1] > d[i][1]) d[i][1] = e.d[i][1];
				}
			}
			/* inline */
			glm::vec3 centroid() const
			{
				return glm::vec3(
					d[0][0] + d[0][1] * 0.5,
					d[1][0] + d[1][1] * 0.5,
					d[2][0] + d[2][1] * 0.5);
			}
			bool Intersect(const float*, const float*, float&, float&, uint8_t&) const;
			float d[kNumPlaneSetNormals][2];
			const SceneObject* obj;
		};

		struct Octree
		{
			Octree(const Extents& sceneExtents)
			{
				float xDiff = sceneExtents.d[0][1] - sceneExtents.d[0][0];
				float yDiff = sceneExtents.d[1][1] - sceneExtents.d[1][0];
				float zDiff = sceneExtents.d[2][1] - sceneExtents.d[2][0];
				float maxDiff = glm::max(xDiff, glm::max(yDiff, zDiff));
				glm::vec3 minPlusMax(
					sceneExtents.d[0][0] + sceneExtents.d[0][1],
					sceneExtents.d[1][0] + sceneExtents.d[1][1],
					sceneExtents.d[2][0] + sceneExtents.d[2][1]);

				bbox[0] = (minPlusMax - maxDiff) * 0.5f;
				bbox[1] = (minPlusMax + maxDiff) * 0.5f;
				root = new OctreeNode;
			}

			~Octree() { deleteOctreeNode(root); }

			void Insert(const Extents* extents) { Insert(root, extents, bbox, 0); }
			void Build() { Build(root, bbox); };

			struct OctreeNode
			{
				OctreeNode* child[8] = { nullptr };
				std::vector<const Extents*> nodeExtentsList; // pointer to the objects extents 
				Extents nodeExtents; // extents of the octree node itself 
				bool isLeaf = true;
			};

			struct QueueElement
			{
				const OctreeNode* node; // octree node held by this element in the queue 
				float t; // distance from the ray origin to the extents of the node 
				QueueElement(const OctreeNode* n, float tn) : node(n), t(tn) {}
				// priority_queue behaves like a min-heap
				friend bool operator < (const QueueElement& a, const QueueElement& b) { return a.t > b.t; }
			};

			OctreeNode* root = nullptr; // make unique son don't have to manage deallocation 
			glm::vec3 bbox[2];

		private:

			void deleteOctreeNode(OctreeNode*& node)
			{
				for (uint8_t i = 0; i < 8; i++) {
					if (node->child[i] != nullptr) {
						deleteOctreeNode(node->child[i]);
					}
				}
				delete node;
			}

			void Insert(OctreeNode*& node, const Extents* extents, const glm::vec3* bbox, uint32_t depth)
			{
				if (node->isLeaf) {
					if (node->nodeExtentsList.size() == 0 || depth == 16) {
						node->nodeExtentsList.push_back(extents);
						CH_TRACE("Leaf");
					}
					else {
						node->isLeaf = false;
						// Re-insert extents held by this node
						while (node->nodeExtentsList.size()) {
							Insert(node, node->nodeExtentsList.back(), bbox, depth);
							node->nodeExtentsList.pop_back();
						}
						// Insert new extent
						Insert(node, extents, bbox, depth);
					}
				}
				else {
					// Need to compute in which child of the current node this extents should
					// be inserted into
					glm::vec3 extentsCentroid = extents->centroid();
					glm::vec3 nodeCentroid = (bbox[0] + bbox[1]) * 0.5f;
					glm::vec3 childBBox[2];
					uint8_t childIndex = 0;
					// x-axis
					if (extentsCentroid.x > nodeCentroid.x) {
						childIndex = 4;
						childBBox[0].x = nodeCentroid.x;
						childBBox[1].x = bbox[1].x;
					}
					else {
						childBBox[0].x = bbox[0].x;
						childBBox[1].x = nodeCentroid.x;
					}
					// y-axis
					if (extentsCentroid.y > nodeCentroid.y) {
						childIndex += 2;
						childBBox[0].y = nodeCentroid.y;
						childBBox[1].y = bbox[1].y;
					}
					else {
						childBBox[0].y = bbox[0].y;
						childBBox[1].y = nodeCentroid.y;
					}
					// z-axis
					if (extentsCentroid.z > nodeCentroid.z) {
						childIndex += 1;
						childBBox[0].z = nodeCentroid.z;
						childBBox[1].z = bbox[1].z;
					}
					else {
						childBBox[0].z = bbox[0].z;
						childBBox[1].z = nodeCentroid.z;
					}

					// Create the child node if it doesn't exsit yet and then insert the extents in it
					if (node->child[childIndex] == nullptr)
						node->child[childIndex] = new OctreeNode;
					Insert(node->child[childIndex], extents, childBBox, depth + 1);
				}
			}

			void Build(OctreeNode*& node, const glm::vec3* bbox)
			{
				if (node->isLeaf) {
					for (const auto& e : node->nodeExtentsList) {
						node->nodeExtents.extendBy(*e);
					}
				}
				else {
					for (uint8_t i = 0; i < 8; ++i) {
						if (node->child[i]) {
							glm::vec3 childBBox[2];
							glm::vec3 centroid = (bbox[0] + bbox[1]) * 0.5f;
							// x-axis
							childBBox[0].x = (i & 4) ? centroid.x : bbox[0].x;
							childBBox[1].x = (i & 4) ? bbox[1].x : centroid.x;
							// y-axis
							childBBox[0].y = (i & 2) ? centroid.y : bbox[0].y;
							childBBox[1].y = (i & 2) ? bbox[1].y : centroid.y;
							// z-axis
							childBBox[0].z = (i & 1) ? centroid.z : bbox[0].z;
							childBBox[1].z = (i & 1) ? bbox[1].z : centroid.z;

							// Inspect child
							Build(node->child[i], childBBox);

							// Expand extents with extents of child
							node->nodeExtents.extendBy(node->child[i]->nodeExtents);
						}
					}
				}
			}
		};

		std::vector<Extents> extentsList;
		Octree* octree = nullptr;
	public:
		BVH(std::vector<std::shared_ptr< SceneObject>>& s);
		bool Intersect(Ray ray, float intersection_eps, IntersectionData* isec_data) const;
		~BVH() { delete octree; }
	};
}