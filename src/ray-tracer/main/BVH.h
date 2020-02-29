#pragma once
#include <ray-tracer/main/Scene.h>

namespace Chroma
{
	struct BVHNode
	{
		BVHNode* left;
		BVHNode* right;

		glm::vec3 extents[2] = { glm::vec3(), glm::vec3() };

		std::vector<SceneObject*>scene_objects;

		BVHNode()
		{
			left = NULL;
			right = NULL;

			extents[0] = glm::vec3();
			extents[1] = glm::vec3();
		}
	};
	class BVH
	{
	public:
		BVH(Scene& scene);

		int m_max_dept = 5;
	private:
		glm::vec3 m_extents[2];

		BVHNode* m_tree_head = NULL;


		void ConstructTree(std::vector<SceneObject*>& scene_objects);
		void ConstructTreeHelper(std::vector<SceneObject*>& scene_objects, BVHNode* node, int dept, int l_range, int u_range);

		bool IsInside(BVHNode* node, SceneObject*);
		
	};
}