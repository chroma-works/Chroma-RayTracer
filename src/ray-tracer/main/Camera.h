#pragma once
#include <thirdparty/glm/glm/glm.hpp>
#include <string>

namespace Chroma
{
	class Camera
	{
	public:
		Camera(float width, float height, float near_clip, float far_clip, float fov);
		//Getters
		inline glm::vec3 GetPosition() { return m_pos; }
		inline glm::vec3 GetGaze() { return m_gaze; }
		inline glm::vec3 GetUp() { return m_up; }
		inline glm::vec2* GetNearPlane() { return m_near_p; }
		inline float GetNearDist() { return m_near_dist; }
		inline unsigned int GetID() { return m_id; }
		inline glm::ivec2 GetResolution() { return m_res; }
		inline std::string GetImageName() { return m_img_name; }

		const glm::mat4& GetProjectionMatrix() const { return m_projection_matrix; }
		const glm::mat4& GetViewMatrix() const { return m_view_matrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_view_projection_matrix; }


		//Setters
		inline void SetPosition(glm::vec3 pos) { m_pos = pos; RecalculateViewMatrix(); }
		inline void SetGaze(glm::vec3 gaze) { m_gaze = gaze; RecalculateViewMatrix(); }
		inline void SetUp(glm::vec3 up) { m_up = up; RecalculateViewMatrix(); }
		inline void SetNearPlane(glm::vec2* plane) { m_near_p[0] = plane[0];  m_near_p[1] = plane[1]; RecalculateProjectionMatrix(); }
		inline void SetNearDist(float d) { m_near_dist = d; RecalculateProjectionMatrix(); }
		inline void SetResolution(glm::ivec2 res) { m_res = res; RecalculateProjectionMatrix(); }
		inline void SetImageName(std::string name) { m_img_name = name; }

	private:
		glm::ivec2 m_res;
		unsigned int m_id;
		glm::vec3 m_pos;
		glm::vec3 m_gaze;
		glm::vec3 m_up;

		glm::vec2 m_near_p[2];
		float m_near_dist;

		std::string m_img_name = "not_set";

		void RecalculateViewMatrix();
		void RecalculateProjectionMatrix();
		void RecalculateUp();

		glm::mat4 m_projection_matrix;
		glm::mat4 m_view_matrix;
		glm::mat4 m_view_projection_matrix;

	};

}

