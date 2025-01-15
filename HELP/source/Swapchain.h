#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vulkan/vulkan.h>

#include "Device.h"

#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

namespace eng {
	class Swapchain {
	public:
		Swapchain(Device &device, const VkExtent2D &windowExtent);
		~Swapchain();

		void recreateSwapchain();

		VkRenderPass getRenderPass() const;
		VkFramebuffer getFramebuffer(std::uint32_t imageIndex) const;
		VkExtent2D getExtent() const;
		VkSemaphore getImageAvailableSemaphore(std::uint32_t currentFrame) const;
		VkSemaphore getRenderFinishedSemaphore(std::uint32_t currentFrame) const;
		VkFence getInFlightFence(std::uint32_t currentFrame) const;
		VkSwapchainKHR getSwapchain() const;

		const int MAX_FRAMES_IN_FLIGHT = 2;
	private:
		void createSwapchain();
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		void cleanupSwapchain();

		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR choosePresentModes(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &capabilities);
		void printPresentMode(const VkPresentModeKHR &presentMode);
		
		VkSwapchainKHR m_swapchain;
		VkRenderPass m_renderPass;
		std::vector<VkImageView> m_imageViews;
		std::vector<VkFramebuffer> m_framebuffers;
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;

		std::vector<VkImage> m_images;
		VkFormat m_imageFormat;
		VkExtent2D m_extent;

		Device &m_device;
		VkExtent2D m_windowExtent;
	};
}

#endif