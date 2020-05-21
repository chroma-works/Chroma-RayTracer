#pragma once
#include <ray-tracer/editor/Logger.h>
#include <thirdparty/glm/glm/glm.hpp>
#include <string>

namespace Chroma
{
	class Camera
	{
	public:
		Camera(float width = 1280, float height = 720, 
			float near_clip = 0.1f, float far_clip = 300.0f, 
			float fov = 60.0f);

		//Getters
		inline glm::vec3 GetPosition() { return m_pos; }
		inline glm::vec3 GetGaze() { return m_gaze; }
		inline glm::vec3 GetUp() { return m_up; }
		inline glm::vec2* GetNearPlane() { return m_near_p; }
		inline float GetNearDist() { return m_near_dist; }
		inline unsigned int GetID() { return m_id; }
		inline glm::ivec2 GetResolution() { return m_res; }
		inline std::string GetImageName() { return m_img_name; }
		inline unsigned int GetNumberOfSamples() { return m_num_samples; }
		inline float GetFocalDistance() { return m_focal_dist; }
		inline float GetApertureSize() { return m_aperture_size; }

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
		inline void SetNumberOfSamples(unsigned int n_samp) 
		{ 
			if ((sqrt(n_samp) - floor(sqrt(n_samp))) != 0)
			{
				CH_WARN("Sample size is not perfect square: Setting to closest square number.");
				int floor_squared = floor(sqrt(n_samp)) * floor(sqrt(n_samp));
				int ceil_squared = ceil(sqrt(n_samp)) * ceil(sqrt(n_samp));

				int dist_floor = n_samp - floor_squared;
				int dis_ceil = ceil_squared - n_samp;
				n_samp = dist_floor <= dis_ceil ? floor_squared : ceil_squared;
			}
			m_num_samples = n_samp; 
		}
		inline void SetFocalDistance(float focal_dist) { m_focal_dist = focal_dist; }
		inline void SetApertureSize(float aperture_size) { m_aperture_size = aperture_size; }

		float m_gamma = 2.2f;
		float m_saturation = 1.0f;
		float m_key_val = 0.18f;
		float m_burn = 1.0f;

	private:
		unsigned int m_id;
		glm::vec3 m_pos;
		glm::vec3 m_gaze;
		glm::vec3 m_up;

		glm::vec2 m_near_p[2];
		float m_near_dist;
		glm::ivec2 m_res;
		unsigned int m_num_samples = 1;

		float m_focal_dist = 20.0f;
		float m_aperture_size = 0.0f;

		std::string m_img_name = "not_set";

		void RecalculateViewMatrix();
		void RecalculateProjectionMatrix();
		void RecalculateUp();

		glm::mat4 m_projection_matrix;
		glm::mat4 m_view_matrix;
		glm::mat4 m_view_projection_matrix;

	};

}

