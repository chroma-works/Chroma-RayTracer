#pragma once
#include <thirdparty\glad\include\glad\glad.h>
#include <thirdparty/glfw-3.3/include/GLFW/glfw3.h>

#include <iostream>
#include <ray-tracer/editor/Logger.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

class Window
{
public:
	Window(int width, int height, std::string title);

	virtual ~Window();

	inline int GetWidth() { return m_data.width; }
	inline int GetHeight() { return m_data.height; }
	inline void SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
		m_vsync = enabled;
	}
	inline bool IsVSync() const{ return m_vsync; }

	inline bool ShouldClose() const { return  glfwWindowShouldClose(m_window_handle); }

	void OnUpdate();
private:
	GLFWwindow* m_window_handle;
	bool m_vsync;

	struct WindowData
	{
		int width;
		int height;
		std::string title;
	}m_data;
};

