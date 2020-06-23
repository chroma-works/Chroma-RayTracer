#pragma once
#include <string>
#include <list>
#include <thirdparty/glm/glm/glm.hpp>

#include "Observer.h"

namespace CHR
{
	enum IM_POST_PROC_T{none = 0, clamp = 1, tone_map = 2 };//none means already ldr
	class Settings : public Subject
	{
	public:
		static Settings* GetInstance() 
		{
			if (!s_instance)
				s_instance = new Settings;
			return s_instance;
		}

		void SetResolution(glm::ivec2 res);
		inline glm::ivec2 GetResolution() const { return m_resolution; }

		void Attach(Observer* observer);
		void Detach(Observer* observer) ;
		void Notify();

		int m_thread_count = 8;
		std::string m_act_editor_cam_name = "";
		std::string m_act_rt_cam_name = "";
		float m_camera_move_speed = 0.4;
		float m_camera_rotate_speed = 0.005;
		float m_shadow_eps = 0.01;
		float m_intersection_eps = 0.000001;
		bool m_calc_shadows = true;
		bool m_calc_reflections = true;
		bool m_calc_refractions = true;
		int m_recur_depth = 6;
		IM_POST_PROC_T m_ldr_post_process = none;
	private:
		Settings(){}
		static Settings* s_instance;

		std::list<Observer*> m_observers;

		glm::ivec2 m_resolution = {800, 800};

	};
}