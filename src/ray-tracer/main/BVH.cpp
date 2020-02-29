#include "BVH.h"
#include <ray-tracer/editor/Logger.h>

namespace Chroma
{

	BVH::BVH(Scene& scene)
	{
		std::vector<SceneObject*> scene_objects;
		glm::vec3 extents[2];
		extents[0] = glm::vec3(INFINITY);
		extents[1] = glm::vec3(-INFINITY);

		//Find the initial bounding box
		for (auto it = scene.m_scene_objects.begin(); it != scene.m_scene_objects.end(); it++)
		{

			extents[0].x = glm::min(it->second->m_mesh.GetMinBound().x, extents[0].x);
			extents[1].x = glm::max(it->second->m_mesh.GetMaxBound().x, extents[1].x);
			extents[0].y = glm::min(it->second->m_mesh.GetMinBound().y, extents[0].y);
			extents[1].y = glm::max(it->second->m_mesh.GetMaxBound().y, extents[1].y);
			extents[0].z = glm::min(it->second->m_mesh.GetMinBound().z, extents[0].z);
			extents[1].z = glm::max(it->second->m_mesh.GetMaxBound().z, extents[1].z);

			scene_objects.push_back(it->second.get());
		}

		m_tree_head = new BVHNode();

		m_tree_head->extents[0] = extents[0];
		m_tree_head->extents[1] = extents[1];

		ConstructTree(scene_objects);
	}

	void BVH::ConstructTree(std::vector<SceneObject*>& scene_objects)
	{
		ConstructTreeHelper(scene_objects, m_tree_head, 0, 0, scene_objects.size());
	}
	void BVH::ConstructTreeHelper(std::vector<SceneObject*>& scene_objects, BVHNode* node, int dept, int l_range, int u_range)
	{
		if (l_range == u_range)//Base case 1
		{
			node->left = NULL;
			node->right = NULL;
			node->scene_objects.clear();

			return;
		}
		else if (l_range + 1 == u_range)//Base case 2
		{
			node->left = NULL;
			node->right = NULL;
			node->scene_objects.push_back(scene_objects[l_range]);

			node->extents[0] = scene_objects[l_range]->m_mesh.GetMinBound();
			node->extents[1] = scene_objects[l_range]->m_mesh.GetMaxBound();

			//CH_TRACE("At dept: " + std::to_string(dept) + scene_objects[l_range]->GetName());

			CH_TRACE("Extents from dept " + std::to_string(dept) + " (" +
				std::to_string(node->extents[0].x) + ", " +
				std::to_string(node->extents[0].y) + ", " +
				std::to_string(node->extents[0].z) + " - (" +
				std::to_string(node->extents[1].x) + ", " +
				std::to_string(node->extents[1].y) + ", " +
				std::to_string(node->extents[1].z) + ")");

			return;

		}
		else if (m_max_dept == dept)//Base case 3
		{
			node->left = NULL;
			node->right = NULL;

			node->extents[0] = glm::vec3(INFINITY);
			node->extents[1] = glm::vec3(-INFINITY);

			for (int j = l_range; j < u_range; j++)
			{
				node->scene_objects.push_back(scene_objects[j]);

				//CH_TRACE("At dept: " + std::to_string(dept) + scene_objects[j]->GetName());

				node->extents[0].x = glm::min(scene_objects[j]->m_mesh.GetMinBound().x, node->extents[0].x);
				node->extents[1].x = glm::max(scene_objects[j]->m_mesh.GetMaxBound().x, node->extents[1].x);
				node->extents[0].y = glm::min(scene_objects[j]->m_mesh.GetMinBound().y, node->extents[0].y);
				node->extents[1].y = glm::max(scene_objects[j]->m_mesh.GetMaxBound().y, node->extents[1].y);
				node->extents[0].z = glm::min(scene_objects[j]->m_mesh.GetMinBound().z, node->extents[0].z);
				node->extents[1].z = glm::max(scene_objects[j]->m_mesh.GetMaxBound().z, node->extents[1].z);
			}

			CH_TRACE("Extents from dept " + std::to_string(dept) + " (" +
				std::to_string(node->extents[0].x) + ", " + 
				std::to_string(node->extents[0].y) + ", " +
				std::to_string(node->extents[0].z) + " - (" + 
				std::to_string(node->extents[1].x) + ", " + 
				std::to_string(node->extents[1].y) + ", " +
				std::to_string(node->extents[1].z) + ")");

			return;
		}
		else
		{
			//Bound the interval
			node->extents[0] = glm::vec3(INFINITY);
			node->extents[1] = glm::vec3(-INFINITY);

			for (int j = l_range;  j < u_range; j++)
			{
				node->extents[0].x = glm::min(scene_objects[j]->m_mesh.GetMinBound().x, node->extents[0].x);
				node->extents[1].x = glm::max(scene_objects[j]->m_mesh.GetMaxBound().x, node->extents[1].x);
				node->extents[0].y = glm::min(scene_objects[j]->m_mesh.GetMinBound().y, node->extents[0].y);
				node->extents[1].y = glm::max(scene_objects[j]->m_mesh.GetMaxBound().y, node->extents[1].y);
				node->extents[0].z = glm::min(scene_objects[j]->m_mesh.GetMinBound().z, node->extents[0].z);
				node->extents[1].z = glm::max(scene_objects[j]->m_mesh.GetMaxBound().z, node->extents[1].z);
			}
			float split;
			int split_axis = dept % 3;//x,y or z
			split = (node->extents[0][split_axis] + node->extents[1][split_axis]) / 2.0f;
			dept++;
			CH_TRACE("Extents from dept " + std::to_string(dept) + " (" +
				std::to_string(node->extents[0].x) + ", " +
				std::to_string(node->extents[0].y) + ", " +
				std::to_string(node->extents[0].z) + ") - (" +
				std::to_string(node->extents[1].x) + ", " +
				std::to_string(node->extents[1].y) + ", " +
				std::to_string(node->extents[1].z) + ")");

			int i = l_range;
			for (int j = l_range; j < u_range; j++)
			{
				if (scene_objects[j]->m_mesh.GetMinBound()[split_axis] < split)
				{
					std::swap(scene_objects[j], scene_objects[i]);
					i++;
				}
			}

			node->left = NULL;
			node->right = NULL;

			if (l_range != i)
			{
				node->left = new BVHNode();
				ConstructTreeHelper(scene_objects, node->left, dept, l_range, i);
			}
			if (u_range != i)
			{
				node->right = new BVHNode();
				ConstructTreeHelper(scene_objects, node->right, dept, i, u_range);
			}

		}
	}
	bool BVH::IsInside(BVHNode* node, SceneObject* object)
	{
		glm::vec3 min_b = object->m_mesh.GetMinBound();
		glm::vec3 max_b = object->m_mesh.GetMaxBound();

		return (min_b.x <= node->extents[1].x && max_b.x >= node->extents[0].x) &&
			(min_b.y <= node->extents[1].y && max_b.y >= node->extents[0].y) &&
			(min_b.z <= node->extents[0].z && max_b.z >= node->extents[0].z);
	}
}
