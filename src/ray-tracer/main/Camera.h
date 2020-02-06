#pragma once
#include <thirdparty/glm/glm/glm.hpp>

namespace CH_RayTracer
{
	class Camera
	{
	public:
		//Getters
		inline glm::vec3 GetPos() { return m_pos; }
		inline glm::vec3 GetGaze() { return m_gaze; }
		inline glm::vec3 GetUp() { return m_up; }
		inline glm::vec3 GetNearPlane() { return m_near_p; }
		inline int GetNearDist() { return m_near_dist; }
		inline unsigned int GetID() { return m_id; }

		//Setters
		inline void SetPos(glm::vec3 pos) { m_pos = pos; }
		inline void SetGaze(glm::vec3 gaze) { m_gaze = gaze; }
		inline void SetUp(glm::vec3 up) { m_up = up; }
		inline void SetNearPlane(glm::vec3 plane) { m_near_p = plane; }
		inline void SetNearDist(int d) { m_near_dist = d; }

		glm::ivec2 m_res;
	private:
		unsigned int m_id;
		glm::vec3 m_pos;
		glm::vec3 m_gaze;
		glm::vec3 m_up;

		glm::vec3 m_near_p;
		int m_near_dist;

	};

}

