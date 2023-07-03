#include "Window.hpp"

namespace ve {
	Window::Window(int width, int height, std::string title)
		: winWidth(width), winHeight(height), winTitle(title) {
		createGLFWWindow();
	}

	Window::~Window() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::createSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a surface.");
		}
	}

	glm::vec2 Window::getFramebufferSize() {
		int framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		return {framebufferWidth, framebufferHeight};
	}

	void Window::createGLFWWindow() {
		if (glfwInit() == GLFW_FALSE) {
			throw std::runtime_error("Failed to initialize GLFW.");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(winWidth, winHeight, winTitle.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pWindow->winWidth = width;
		pWindow->winHeight = height;
	}
}