#include "Device.h"

namespace eng {
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	Device::Device(Window &window)
		: m_window(window) {
		createInstance();
		createDebugMessenger();
		createWindowSurface();
		choosePhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	Device::~Device() {
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);

		vkDestroyDevice(m_device, nullptr);

		if (m_enableValidationLayers) {
			destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

		vkDestroyInstance(m_instance, nullptr);
	}

	void Device::createInstance() {
		if (m_enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("Required validation layers are not supported.");
		}

		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.apiVersion = VK_API_VERSION_1_0;
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pApplicationName = "Vulkan Application";
		applicationInfo.pEngineName = "Vulkan Engine";

		std::vector<const char *> requiredExtensions = getRequiredExtensions();
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = getDebugUtilsMessengerCreateInfo();

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.flags = NULL;
		instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size());
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
		if (m_enableValidationLayers) {
			instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(m_validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();
			instanceCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugUtilsMessengerCreateInfo);
		} else {
			instanceCreateInfo.enabledLayerCount = 0;
			instanceCreateInfo.ppEnabledLayerNames = nullptr;
			instanceCreateInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create instance.");
		}

		printExtensionData(requiredExtensions);
	}

	void Device::createDebugMessenger() {
		if (!m_enableValidationLayers) {
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = getDebugUtilsMessengerCreateInfo();

		if (createDebugUtilsMessengerEXT(m_instance, &debugUtilsMessengerCreateInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create debug messenger.");
		}
	}

	void Device::createWindowSurface() {
		m_window.createWindowSurface(m_instance, m_surface);
	}

	void Device::choosePhysicalDevice() {
		std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices();
		if (physicalDevices.empty()) {
			throw std::runtime_error("Failed to find a GPU with Vulkan support.");
		}

		for (const VkPhysicalDevice &physicalDevice : physicalDevices) {
			if (isPhysicalDeviceSuitable(physicalDevice)) {
				m_physicalDevice = physicalDevice;
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU.");
		}

		printChosenPhysicalDevice();
	}

	void Device::createLogicalDevice() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

		float queuePriorities = 1.0f;
		std::set<std::uint32_t> uniqueQueueFamilies = {
			queueFamilyIndices.graphicsFamilyIndex.value(),
			queueFamilyIndices.presentFamilyIndex.value()
		};

		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

		for (const std::uint32_t &uniqueQueueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
			deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfo.pNext = nullptr;
			deviceQueueCreateInfo.pQueuePriorities = &queuePriorities;
			deviceQueueCreateInfo.queueCount = 1;
			deviceQueueCreateInfo.queueFamilyIndex = uniqueQueueFamily;

			deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<std::uint32_t>(deviceQueueCreateInfos.size());
		deviceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(m_deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		if (m_enableValidationLayers) {
			deviceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(m_validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();
		} else {
			deviceCreateInfo.enabledLayerCount = 0;
			deviceCreateInfo.ppEnabledLayerNames = nullptr;
		}

		if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create device.");
		}

		vkGetDeviceQueue(m_device, queueFamilyIndices.graphicsFamilyIndex.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, queueFamilyIndices.presentFamilyIndex.value(), 0, &m_presentQueue);
	}

	void Device::createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamilyIndex.value();

		if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool.");
		}
	}

	std::vector<const char *> Device::getRequiredExtensions() {
		std::uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_enableValidationLayers) {
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return requiredExtensions;
	}

	std::vector<VkExtensionProperties> Device::getAvailableExtensions() {
		std::uint32_t availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

		return availableExtensions;
	}

	void Device::printExtensionData(const std::vector<const char *> &requiredExtensions) {
		std::vector<VkExtensionProperties> availableExtensions = getAvailableExtensions();
		
		std::cout << "Available Extensions:\n";
		for (const VkExtensionProperties &availableExtensionProperties : availableExtensions) {
			std::cout << '\t' << availableExtensionProperties.extensionName << '\n';
		}

		std::cout << "Required Extensions:\n";
		for (const char *requiredExtension : requiredExtensions) {
			std::cout << '\t' << requiredExtension << '\n';
		}
	}

	std::vector<VkLayerProperties> Device::getAvailableLayers() {
		std::uint32_t availableLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(availableLayerCount);
		vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

		return availableLayers;
	}

	bool Device::checkValidationLayerSupport() {
		std::vector<VkLayerProperties> availableLayers = getAvailableLayers();

		for (const char *validationLayer : m_validationLayers) {
			bool found = false;
			for (const VkLayerProperties &layerProperty : availableLayers) {
				if (std::strcmp(validationLayer, layerProperty.layerName) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				return false;
			}
		}

		return true;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Device::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) {
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::cout << callbackData->pMessage << '\n';
		}
		
		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT Device::getDebugUtilsMessengerCreateInfo() {
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
		debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugUtilsMessengerCreateInfo.pNext = nullptr;
		debugUtilsMessengerCreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugUtilsMessengerCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugUtilsMessengerCreateInfo.pfnUserCallback = debugCallback;
		debugUtilsMessengerCreateInfo.pUserData = nullptr;

		return debugUtilsMessengerCreateInfo;
	}

	Device::SwapchainSupportDetails Device::querySwapchainSupport(const VkPhysicalDevice &physicalDevice) {
		SwapchainSupportDetails swapchainSupportDetails{};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &swapchainSupportDetails.capabilities);

		std::uint32_t surfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &surfaceFormatCount, nullptr);
		if (surfaceFormatCount != 0) {
			swapchainSupportDetails.formats.resize(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &surfaceFormatCount, swapchainSupportDetails.formats.data());
		}

		std::uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			swapchainSupportDetails.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, swapchainSupportDetails.presentModes.data());
		}

		return swapchainSupportDetails;
	}

	Device::SwapchainSupportDetails Device::querySwapchainSupport() {
		return querySwapchainSupport(m_physicalDevice);
	}

	VkSurfaceKHR Device::getSurface() const {
		return m_surface;
	}

	VkDevice Device::getDevice() const {
		return m_device;
	}

	VkCommandPool Device::getCommandPool() const {
		return m_commandPool;
	}

	VkQueue Device::getGraphicsQueue() const {
		return m_graphicsQueue;
	}

	VkQueue Device::getPresentQueue() const {
		return m_presentQueue;
	}

	std::vector<VkPhysicalDevice> Device::getPhysicalDevices() {
		std::uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

		return physicalDevices;
	}

	bool Device::isPhysicalDeviceSuitable(const VkPhysicalDevice &physicalDevice) {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		bool extensionsSupported = checkPhysicalDeviceExtensionSupport(physicalDevice);

		bool swapchainSufficient = false;
		if (extensionsSupported) {
			SwapchainSupportDetails swapchainSupportDetails = querySwapchainSupport(physicalDevice);
			swapchainSufficient = !swapchainSupportDetails.formats.empty() && !swapchainSupportDetails.presentModes.empty();
		}

		return queueFamilyIndices.isComplete() && extensionsSupported && swapchainSufficient;
	}

	void Device::printChosenPhysicalDevice() {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

		std::string physicalDeviceType = "";
		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			physicalDeviceType = "discrete";
		} else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			physicalDeviceType = "integrated";
		}

		std::cout << "Chosen " << physicalDeviceType << " GPU: " << physicalDeviceProperties.deviceName << '\n';
	}

	bool Device::QueueFamilyIndices::isComplete() const {
		return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
	}

	Device::QueueFamilyIndices Device::findQueueFamilies(const VkPhysicalDevice &physicalDevice) {
		QueueFamilyIndices queueFamilyIndices{};

		std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilies(physicalDevice);

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				queueFamilyIndices.graphicsFamilyIndex = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_surface, &presentSupport);
			if (presentSupport) {
				queueFamilyIndices.presentFamilyIndex = i;
			}

			if (queueFamilyIndices.isComplete()) {
				break;
			}

			++i;
		}

		return queueFamilyIndices;
	}

	Device::QueueFamilyIndices Device::findQueueFamilies() {
		return findQueueFamilies(m_physicalDevice);
	}

	void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer.");
		}

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate vertex buffer memory.");
		}

		vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
	}

	std::uint32_t Device::findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &physicalDeviceMemoryProperties);
		
		for (std::uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i) {
			if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}

	std::vector<VkQueueFamilyProperties> Device::getQueueFamilies(const VkPhysicalDevice &physicalDevice) {
		std::uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		return queueFamilies;
	}

	bool Device::checkPhysicalDeviceExtensionSupport(const VkPhysicalDevice &physicalDevice) {
		std::vector<VkExtensionProperties> availableExtensions = getAvailablePhysicalDeviceExtensions(physicalDevice);

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

		for (const VkExtensionProperties& availableExtension : availableExtensions) {
			requiredExtensions.erase(availableExtension.extensionName);
		}

		return requiredExtensions.empty();
	}

	std::vector<VkExtensionProperties> Device::getAvailablePhysicalDeviceExtensions(const VkPhysicalDevice &physicalDevice) {
		std::uint32_t availableExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());
		
		return availableExtensions;
	}
}