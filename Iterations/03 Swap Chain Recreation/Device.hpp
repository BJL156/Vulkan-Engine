#pragma once

#include <VULKAN/vulkan.h>
#include <GLFW/glfw3.h>

#include "Window.hpp"

#include <set>
#include <optional>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>

namespace ve {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Device {
	public:
		Device(Window& win);
		~Device();

		VkSurfaceKHR getSurface() { return surface; }
		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
		VkDevice getDevice() { return device; }
		VkCommandPool getCommandPool() { return commandPool; }
		VkQueue getGraphicsQueue() { return graphicsQueue; }
		VkQueue getPresentQueue() { return presentQueue; }
	private:
		void createDevice();
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		std::vector<const char*> getRequiredExtensions();
		bool checkValidationLayerSupport();
		VkDebugUtilsMessengerCreateInfoEXT fillDebugUtilsMessengerInfo();
		bool isPhysicalDeviceSuitable(VkPhysicalDevice physDevice);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice);
		bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physDevice);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physDevice);

		Window& window;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkCommandPool commandPool;

		const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
		const std::vector<const char*> DEVICE_EXTENSIONS = {"VK_KHR_swapchain"};
	};
}