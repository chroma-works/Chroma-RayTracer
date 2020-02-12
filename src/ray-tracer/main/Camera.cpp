#include "Camera.h"
#include <thirdparty/glm//glm/gtc/matrix_transform.hpp>

static int last_id = 1;

namespace Chroma
{
	Camera::Camera(float width, float height, float near_clip, float far_clip, float fov)
	{
		m_projection_matrix = glm::perspective(glm::radians(fov), (width / height), near_clip, far_clip);
		m_view_matrix = glm::mat4(1.0f);
		m_view_projection_matrix = m_projection_matrix * m_view_matrix;
	}
	void Camera::RecalculateViewMatrix()
	{
		m_view_matrix = glm::lookAt(m_pos, m_gaze + m_pos, m_up);
		m_view_projection_matrix = m_projection_matrix * m_view_matrix;
	}
}