#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>

#include <stdexcept>
#include <string>

namespace ve {
	class Window {
	public:
		Window(int width, int height, std::string title);
		~Window();

		bool shouldClose() { return glfwWindowShouldClose(window); }
		void pollEvents() { glfwPollEvents(); }

		void createSurface(VkInstance instance, VkSurfaceKHR* surface);

		glm::vec2 getFramebufferSize();
	private:
		void createGLFWWindow();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		int winWidth, winHeight;
		std::string winTitle;
		GLFWwindow* window;
	};
}