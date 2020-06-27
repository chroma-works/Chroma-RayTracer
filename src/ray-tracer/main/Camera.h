#pragma once
#include <ray-tracer/editor/Logger.h>
#include <ray-tracer/editor/ImGuiDrawable.h>
#include <ray-tracer/editor/Settings.h>

#include <thirdparty/glm/glm/glm.hpp>

#include <string>

namespace CHR
{
	class Camera : public ImGuiDrawable, public Observer
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
		inline bool IsImportanceSamplingOn() { return m_render_params[0]; }
		inline bool IsNextEventEstimationOn() { return m_render_params[1]; }
		inline bool IsRussianRouletteOn() { return m_render_params[2]; }

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

		inline void SetImportanceSampling( bool is) { m_render_params[0] = is; }
		inline void SetNextEventEstimation( bool nee) { m_render_params[1] = nee; }
		inline void SetRussianRoulette(bool rr) { m_render_params[2] = rr; }

		inline void DrawGUI()
		{
			ImGui::PushStyleColor(ImGuiCol_Header, CHR_COLOR::C_FRAME);
			ImGui::CollapsingHeader(std::string("Camera: " + m_name).c_str(), ImGuiTreeNodeFlags_Leaf);
			ImGui::PopStyleColor();

			static bool flag = true;
			if (flag)
				ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::CollapsingHeader("Transform"))
			{

				if (ImGui::Button("P##1"))SetPosition(glm::vec3());
				ImGui::SameLine();
				glm::vec3 tmp_pos = GetPosition();
				ImGui::DragFloat3("##4", &(tmp_pos.x), 0.05f, 0, 0, "%.3f");
				SetPosition(tmp_pos);

				if (ImGui::Button("G##2"))SetGaze(glm::vec3());
				ImGui::SameLine();
				glm::vec3 tmp_gaze = GetGaze();
				ImGui::DragFloat3("##5", &(tmp_gaze.x), 0.05f, 0, 0, "%.3f");
				SetGaze(glm::normalize(tmp_gaze));

				if (ImGui::Button("U##3"))SetUp(glm::vec3(0.0, 1.0, 0.0));
				ImGui::SameLine();
				glm::vec3 tmp_up = GetUp();
				ImGui::DragFloat3("##6", &(tmp_up.x), 0.05f, 0, 0, "%.3f");
				SetUp(glm::normalize(tmp_up));
				ImGui::Separator();
			}
			if (flag)
				ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::CollapsingHeader("Lens"))
			{

				float tmp_f = GetFocalDistance();
				ImGui::DragFloat("Focal Dist.", &tmp_f, 0.05f, 0.0f, INFINITY, "%.3f");
				SetFocalDistance(tmp_f);

				float tmp_a = GetApertureSize();
				ImGui::DragFloat("Apert. Size", &tmp_a, 0.05f, 0.0f, INFINITY, "%.3f");
				SetApertureSize(tmp_a);
				ImGui::Separator();
			}

			if (flag)
				ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::CollapsingHeader("Tone Mapping Operator"))
			{
				ImGui::DragFloat("Key Value", &m_key_val, 0.005f, 0.0, 0.40f);
				ImGui::DragFloat("Burn percentage", &m_burn_perc, 0.05f, 0.0, 100.0f);
				ImGui::DragFloat("Saturation", &m_saturation, 0.05f, 0.0f, 1.0f);
				ImGui::DragFloat("Gamma", &m_gamma, 0.05f, 1.0f, 3.0f);
				ImGui::Separator();
			}


			char* tmp_buf = strdup(GetImageName().c_str());
			ImGui::InputText("Image Name", tmp_buf, 20, 0);
			SetImageName(tmp_buf);

			flag = false;
		}

		inline void GetNotified()
		{
			m_res = Settings::GetInstance()->GetResolution();
		}

		float m_gamma = 2.2f;
		float m_saturation = 1.0f;
		float m_key_val = 0.18f;
		float m_burn_perc = 1.0f;

		bool m_left_handed = false;


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

		std::string m_img_name = "not set";
		bool m_render_params[3] = { false, false, false };

		void RecalculateViewMatrix();
		void RecalculateProjectionMatrix();
		void RecalculateUp();

		glm::mat4 m_projection_matrix;
		glm::mat4 m_view_matrix;
		glm::mat4 m_view_projection_matrix;

	};

}

