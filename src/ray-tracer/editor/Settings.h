#pragma once
#include <string>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	enum IM_POST_PROC_T{clamp = 0, tone_map=1 };
	struct Settings
	{
		int thread_count = 8;
		std::string act_editor_cam_name = "";
		std::string act_rt_cam_name = "";
		glm::ivec2 resolution = {800, 800};
		float camera_move_speed = 0.4;
		float camera_rotate_speed = 0.005;
		float shadow_eps = 0.01;
		float intersection_eps = 0.000001;
		bool calc_shadows = true;
		bool calc_reflections = true;
		bool calc_refractions = true;
		int recur_depth = 6;
		bool save_exr = false;
		IM_POST_PROC_T ldr_post_process = clamp;
	};
}