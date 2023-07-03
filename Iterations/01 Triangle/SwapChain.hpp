#pragma once

#include <VULKAN/vulkan.h>
#include <GLM/glm.hpp>

#include "Device.hpp"
#include "Window.hpp"

#include <vector>
#include <algorithm>
#include <iostream>

namespace ve {
	class SwapChain {
	public:
		SwapChain(Device& dev, Window& win);
		~SwapChain();

		VkResult acquireNextImage(uint32_t& imageIndex);
		VkResult submitCommandBuffers(VkCommandBuffer& commandBuffer, uint32_t& imageIndex);

		VkExtent2D getExtent() { return swapChainExtent; }
		VkRenderPass getRenderPass() { return renderPass; }
		std::vector<VkFramebuffer> getFramebuffers() { return swapChainFramebuffers; }
		VkSwapchainKHR getSwapChain() { return swapChain; }
	private:
		void createSwapChain();
		void createSwapChainKHR();
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkRenderPass renderPass;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;

		Device& device;
		Window& window;
	};
}