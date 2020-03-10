#pragma once
#include <string>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	struct Settings
	{
		int thread_count = 8;
		std::string act_editor_cam_name = "";
		std::string act_rt_cam_name = "";
		glm::ivec2 resolution = {800, 800};
		float camera_move_speed = 0.4;
		float camera_rotate_speed = 0.005;
		float shadow_eps = 0.001;
		float intersection_eps = 0.000001;
		bool calc_shadows = true;
		bool calc_reflections = true;
		bool calc_refractions = true;
	};
}