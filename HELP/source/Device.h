#ifndef DEVICE_H
#define DEVICE_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Window.h"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <optional>
#include <set>

namespace eng {
	class Device {
	public:
		Device(Window& window);
		~Device();

		Device(const Device &) = delete;
		void operator=(const Device &) = delete;
		Device(Device &&) = delete;
		Device &operator=(Device &&) = delete;
		
		struct SwapchainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice &physicalDevice);
		SwapchainSupportDetails querySwapchainSupport();

		struct QueueFamilyIndices {
			std::optional<std::uint32_t> graphicsFamilyIndex;
			std::optional<std::uint32_t> presentFamilyIndex;

			bool isComplete() const;
		};

		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &physicalDevice);
		QueueFamilyIndices findQueueFamilies();
		
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
		std::uint32_t findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties);

		VkSurfaceKHR getSurface() const;
		VkDevice getDevice() const;
		VkCommandPool getCommandPool() const;
		VkQueue getGraphicsQueue() const;
		VkQueue getPresentQueue() const;
	private:
		void createInstance();
		void createDebugMessenger();
		void createWindowSurface();
		void choosePhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		std::vector<const char*> getRequiredExtensions();
		std::vector<VkExtensionProperties> getAvailableExtensions();
		void printExtensionData(const std::vector<const char *> &requiredExtensions);

		std::vector<VkLayerProperties> getAvailableLayers();
		bool checkValidationLayerSupport();
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
			void *userData);
		VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo();

		std::vector<VkPhysicalDevice> getPhysicalDevices();
		bool isPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice);
		void printChosenPhysicalDevice();
		std::vector<VkQueueFamilyProperties> getQueueFamilies(const VkPhysicalDevice &physicalDevice);
		bool checkPhysicalDeviceExtensionSupport(const VkPhysicalDevice &physicalDevice);
		std::vector<VkExtensionProperties> getAvailablePhysicalDeviceExtensions(const VkPhysicalDevice &physicalDevice);

		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device;
		VkCommandPool m_commandPool;

		const std::vector<const char *> m_validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char *> m_deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

#ifdef NDEBUG
		bool m_enableValidationLayers = false;
#else
		bool m_enableValidationLayers = true;
#endif

		Window &m_window;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
	};
}

#endif