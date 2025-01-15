#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Window.h"
#include "Device.h"
#include "Swapchain.h"
#include "Model.h"

#include <vector>
#include <stdexcept>
#include <memory>

namespace eng {
	class Renderer {
	public:
		Renderer(Window &window, Device &device);
		~Renderer();

		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapchainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapchainRenderPass(VkCommandBuffer commandBuffer);

		Swapchain& getSwapchain();
	private:
		void createCommandBuffers();
		void freeCommandBuffers();

		std::uint32_t m_currentFrame = 0;
		std::uint32_t m_imageIndex = 0;

		Window &m_window;
		Device &m_device;
		Swapchain m_swapchain{ m_device, m_window.getExtent() };
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
}

#endif