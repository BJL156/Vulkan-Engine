#include "Device.hpp"

namespace ve {
#if NDEBUG
	const bool ENABLE_VALIDATION_LAYERS = false;
#else
	const bool ENABLE_VALIDATION_LAYERS = true;
#endif

	VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			std::cerr << pCallbackData->pMessage << std::endl;
		}

		// Always return VK_FALSE this is only really used to test the validation layers themselves
		return VK_FALSE;
	}

	VkResult createDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
		PFN_vkCreateDebugUtilsMessengerEXT function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (function == nullptr) {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	void destroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		PFN_vkDestroyDebugUtilsMessengerEXT function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (function != nullptr) {
			function(instance, debugMessenger, pAllocator);
		}
	}

	Device::Device(Window& win)
		: window(win) {
		createDevice();
	}

	Device::~Device() {
		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyDevice(device, nullptr);

		if (ENABLE_VALIDATION_LAYERS) {
			destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);

		vkDestroyInstance(instance, nullptr);
	}

	uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (i << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type");
	}

	void Device::createDevice() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	void Device::createInstance() {
		if (!checkValidationLayerSupport() && ENABLE_VALIDATION_LAYERS) {
			throw std::runtime_error("The requested validation layers that were requested are not supported.");
		}

		// Tell Vulkan driver about the specifics of the program
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Orange Juice Yucky";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "ve";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		// Instance connects program and the Vulkan library
		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pNext = nullptr;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.flags = 0;
		std::vector<const char*> requiredExtensions = getRequiredExtensions();
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		if (ENABLE_VALIDATION_LAYERS) {
			instanceInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			instanceInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

			debugMessengerInfo = fillDebugUtilsMessengerInfo();
			instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugMessengerInfo;
		} else {
			instanceInfo.enabledLayerCount = 0;
			instanceInfo.ppEnabledLayerNames = nullptr;
		}

		if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create an instance.");
		}

		// Get and print all available and required extensions
		uint32_t availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

		std::cout << "Available Vulkan extensions (" << availableExtensions.size() << "):\n";
		for (const VkExtensionProperties& availableExtension : availableExtensions) {
			std::cout << '\t' << availableExtension.extensionName << std::endl;
		}

		std::cout << "Required Vulkan extensions (" << requiredExtensions.size() << "):\n";
		for (const char* requiredExtension : requiredExtensions) {
			std::cout << '\t' << requiredExtension << std::endl;
		}
	}

	void Device::setupDebugMessenger() {
		if (!ENABLE_VALIDATION_LAYERS) {
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = fillDebugUtilsMessengerInfo();

		if (createDebugUtilsMessengerEXT(instance, &debugMessengerInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Failed to setup debug messenger.");
		}
	}

	void Device::createSurface() {
		window.createSurface(instance, &surface);
	}

	void Device::pickPhysicalDevice() {
		// Get all physical devices with Vulkan support
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		if (physicalDevices.empty()) {
			throw std::runtime_error("Failed to find a physical device with Vulkan support.");
		}

		// Check if the physical device is suitable
		for (const VkPhysicalDevice& physDevice : physicalDevices) {
			if (isPhysicalDeviceSuitable(physDevice)) {
				physicalDevice = physDevice;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed find a suitable physical device.");
		}

		// Print selected physical device
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		std::cout << "Picked physical device: " << physicalDeviceProperties.deviceName << std::endl;
	}

	void Device::createLogicalDevice() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		// Select how many queues for each queue family
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo deviceQueueInfo{};
			deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueInfo.pNext = nullptr;
			deviceQueueInfo.queueFamilyIndex = queueFamily;
			deviceQueueInfo.queueCount = 1;
			deviceQueueInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(deviceQueueInfo);
		}

		// get physical device features
		VkPhysicalDeviceFeatures physicalDeviceFeatures{};

		// Logical Device allows the program to make the graphics card do things
		VkDeviceCreateInfo deviceInfo{};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceInfo.pEnabledFeatures = &physicalDeviceFeatures;
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
		deviceInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
		if (ENABLE_VALIDATION_LAYERS) {
			deviceInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			deviceInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		} else {
			deviceInfo.enabledLayerCount = 0;
			deviceInfo.ppEnabledLayerNames = nullptr;
		}

		if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a device.");
		}

		vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	}

	void Device::createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a command pool.");
		}
	}

	std::vector<const char*> Device::getRequiredExtensions() {
		uint32_t requiredExtensionsCount = 0;
		const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionsCount);

		std::vector<const char*> requiredExtensions(glfwRequiredExtensions, glfwRequiredExtensions + requiredExtensionsCount);

		// Only required if validation layers are enabled
		if (ENABLE_VALIDATION_LAYERS) {
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return requiredExtensions;
	}

	bool Device::checkValidationLayerSupport() {
		// Get all available layers
		uint32_t availableLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(availableLayerCount);
		vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

		// Find out if every validation layer is available
		for (const char* layerName : VALIDATION_LAYERS) {
			bool layerFound = false;

			for (const VkLayerProperties& availableLayer : availableLayers) {
				if (strcmp(layerName, availableLayer.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	VkDebugUtilsMessengerCreateInfoEXT Device::fillDebugUtilsMessengerInfo() {
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerInfo.pNext = nullptr;
		debugMessengerInfo.flags = 0;
		debugMessengerInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessengerInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessengerInfo.pfnUserCallback = debugMessageCallback;
		debugMessengerInfo.pUserData = nullptr;

		return debugMessengerInfo;
	}

	bool Device::isPhysicalDeviceSuitable(VkPhysicalDevice physDevice) {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physDevice);

		bool deviceExtensionsSupported = checkPhysicalDeviceExtensionSupport(physDevice);
		bool swapChainSufficient = false;
		if (deviceExtensionsSupported) {
			SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(physDevice);
			swapChainSufficient = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
		}

		return queueFamilyIndices.isComplete() && deviceExtensionsSupported && swapChainSufficient;
	}

	QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice physDevice) {
		QueueFamilyIndices queueFamilyIndices;

		// Get all queue families
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilyProperties.data());

		// Find queue family that has VK_QUEUE_GRAPHICS_BIT supported
		int i = 0;
		for (const VkQueueFamilyProperties& queueFamilyProperty : queueFamilyProperties) {
			if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				queueFamilyIndices.graphicsFamily = i;
			}

			VkBool32 presentQueueSupported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, i, surface, &presentQueueSupported);

			if (presentQueueSupported) {
				queueFamilyIndices.presentFamily = i;
			}

			if (queueFamilyIndices.isComplete()) {
				break;
			}

			i++;
		}

		return queueFamilyIndices;
	}

	bool Device::checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physDevice) {
		// Get all physical device extensions
		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &deviceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &deviceExtensionCount, availableExtensions.data());

		// Be able to easily tick off the extensions that are available with a set
		std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

		for (const VkExtensionProperties& availableExtension : availableExtensions) {
			requiredExtensions.erase(availableExtension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice physDevice) {
		SwapChainSupportDetails supportDetails;

		// Set surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &supportDetails.capabilities);

		// Set surface format
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			supportDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, supportDetails.formats.data());
		}

		// Set surface present modes
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			supportDetails.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, supportDetails.presentModes.data());
		}

		return supportDetails;
	}
}