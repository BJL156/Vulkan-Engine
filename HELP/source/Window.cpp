#include "Window.h"

namespace eng {
    Window::Window(const std::uint32_t &width, const std::uint32_t &height, const std::string name)
        : m_width(width), m_height(height), m_name(name) {
        createGlfwWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    bool Window::shouldClose() {
        return glfwWindowShouldClose(m_window);
    }

    void Window::update() {
        pollEvents();
        updateFPS();
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    void Window::updateFPS() {
        m_frameCount++;

        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = currentFrameTime - m_lastFrameTime;

        if (duration.count() >= 1.0f) {
            float fps = static_cast<float>(m_frameCount) / duration.count();

            std::string newName = m_name + " FPS: " + std::to_string(fps);
            glfwSetWindowTitle(m_window, newName.c_str());

            m_frameCount = 0;
            m_lastFrameTime = currentFrameTime;
        }
    }

    void Window::createWindowSurface(const VkInstance &instance, VkSurfaceKHR &surface) {
        if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface.");
        }
    }

    void Window::resetResizeFlag() {
        m_framebufferResized = false;
    }

    VkExtent2D Window::getExtent() const {
        return {
            m_width, m_height
        };
    }

    bool Window::getResizeFlag() const {
        return m_framebufferResized;
    }

    std::uint32_t Window::getWidth() const {
        return m_width;
    }

    std::uint32_t Window::getHeight() const {
        return m_height;
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        Window *p_window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        p_window->m_width = width;
        p_window->m_height = height;
        p_window->m_framebufferResized = true;
    }

    void Window::createGlfwWindow() {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("Failed to create a GLFW window.");
        }

        glfwSwapInterval(0);

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    }
}