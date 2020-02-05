#include "Window.h"
#include <thirdparty/glfw-3.3/include/GLFW/glfw3.h>


namespace CH_Editor {
	static bool s_GLFW_init = false;

	Window::Window(int width, int height, std::string title)
	{
		if (!s_GLFW_init)
		{
			s_GLFW_init = true;
			int success = glfwInit();
			if (success == 0)
			{
				CH_FATAL("Failed to create GLFW window");
				glfwTerminate();
			}
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		}
		m_data.width = width;
		m_data.height = height;
		m_data.title = title;
		m_window_handle = glfwCreateWindow((int)width, (int)height, title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_window_handle, &m_data);

		glfwMakeContextCurrent(m_window_handle);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			CH_ERROR(false, "Failed to initialize GLAD");
		}

		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_window_handle, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.width = width;
			data.height = height;

		});

		glfwSetWindowCloseCallback(m_window_handle, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		});

		glfwSetKeyCallback(m_window_handle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				//CH_TRACE((event).ToString());
				break;
			}
			case GLFW_RELEASE:
			{
				//CH_TRACE((event).ToString());
				break;
			}
			case GLFW_REPEAT:
			{
				//CH_TRACE((event).ToString());
				break;
			}
			}
		});

		glfwSetCharCallback(m_window_handle, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

		});

		glfwSetMouseButtonCallback(m_window_handle, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{

				break;
			}
			case GLFW_RELEASE:
			{

				break;
			}
			}
		});

		glfwSetScrollCallback(m_window_handle, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

		});

		glfwSetCursorPosCallback(m_window_handle, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		});

		std::string opengl_info = std::string();
		opengl_info = std::string("OpenGL Info:\n") + std::string("\tVendor: ") + std::string((char*)glGetString(GL_VENDOR)) + "\n"
			+ std::string("\tRenderer: ") + std::string((char*)glGetString(GL_RENDERER)) + "\n" + std::string("\tVersion: ")
			+ std::string((char*)glGetString(GL_VERSION));

		CH_INFO(opengl_info);
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_window_handle);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window_handle);
	}
}