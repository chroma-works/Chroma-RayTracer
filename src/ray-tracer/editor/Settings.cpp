#include "Settings.h"

namespace CHR
{
	Settings* Settings::s_instance;

	void Settings::SetResolution(glm::ivec2 res)
	{
		m_resolution = res;
		Notify();
	}
	void Settings::Attach(Observer* observer)
	{
		m_observers.push_back(observer);
	}
	void Settings::Detach(Observer* observer)
	{
		m_observers.remove(observer);
	}
	void Settings::Notify()
	{
		for (auto obs : m_observers)
		{
			obs->GetNotified();
		}
	}
}