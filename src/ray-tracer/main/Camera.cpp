#include "Camera.h"
#include <thirdparty/glm//glm/gtc/matrix_transform.hpp>

static int last_id = 1;

namespace Chroma
{
	Camera::Camera(float width, float height, float near_clip, float far_clip, float fov)
	{
		m_projection_matrix = glm::perspective(glm::radians(fov), ((float)m_res.x / (float)m_res.y), near_clip, far_clip);
		m_near_dist = near_clip;
		m_view_matrix = glm::mat4(1.0f);
		m_view_projection_matrix = m_projection_matrix * m_view_matrix;
	}
	void Camera::RecalculateProjectionMatrix()
	{
		float aspect_ratio = glm::abs(m_near_p[0].x - m_near_p[1].x) / glm::abs(m_near_p[0].y - m_near_p[1].y);
		float fovy = atan(aspect_ratio * glm::abs(m_near_p[0].x - m_near_p[1].x) * 0.5f);
		m_projection_matrix = glm::perspective(fovy,
			16.0f/9.0f
			, m_near_dist, 3000.0f);
		m_view_projection_matrix = m_projection_matrix * m_view_matrix;
	}
	void Camera::RecalculateViewMatrix()
	{
		m_view_matrix = glm::lookAt(m_pos, m_gaze + m_pos, m_up);
		m_view_projection_matrix = m_projection_matrix * m_view_matrix;
	}
	void Camera::RecalculateUp()
	{
		glm::vec3 u = glm::normalize(glm::cross(-m_gaze, m_up));
		SetUp(glm::normalize(glm::cross(u, -m_gaze)));
	}
}