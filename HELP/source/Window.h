#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>
#include <stdexcept>
#include <chrono>

namespace eng {
	class Window {
	public:
		Window(
			const std::uint32_t& width,
			const std::uint32_t& height,
			const std::string name);
		~Window();

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;

		bool shouldClose();

		void update();

		void createWindowSurface(const VkInstance &instance, VkSurfaceKHR &surface);
		
		void resetResizeFlag();

		VkExtent2D getExtent() const;
		bool getResizeFlag() const;
		std::uint32_t getWidth() const;
		std::uint32_t getHeight() const;
	private:
		static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

		void createGlfwWindow();

		void pollEvents();
		void updateFPS();

		GLFWwindow *m_window;
		std::uint32_t m_width, m_height;
		std::string m_name;

		bool m_framebufferResized = false;

		int m_frameCount = 0;
		std::chrono::high_resolution_clock::time_point m_lastFrameTime = std::chrono::high_resolution_clock::now();
	};
}

#endif