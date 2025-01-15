#include "Swapchain.h"

namespace eng {
    Swapchain::Swapchain(Device &device, const VkExtent2D &windowExtent)
        : m_device(device), m_windowExtent(windowExtent) {
        createSwapchain();
        createImageViews();
        createRenderPass();
        createFramebuffers();
        createSyncObjects();
    }

    Swapchain::~Swapchain() {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(m_device.getDevice(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_device.getDevice(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_device.getDevice(), m_inFlightFences[i], nullptr);
        }

        vkDestroyRenderPass(m_device.getDevice(), m_renderPass, nullptr);

        cleanupSwapchain();
    }

    void Swapchain::recreateSwapchain() {
        vkDeviceWaitIdle(m_device.getDevice());

        cleanupSwapchain();

        createSwapchain();
        createImageViews();
        createFramebuffers();
    }

    VkRenderPass Swapchain::getRenderPass() const {
        return m_renderPass;
    }

    VkFramebuffer Swapchain::getFramebuffer(std::uint32_t imageIndex) const {
        if (imageIndex >= m_framebuffers.size()) {
            throw std::runtime_error("Failed to get framebuffer with the image index.");
        }

        return m_framebuffers[imageIndex];
    }

    VkExtent2D Swapchain::getExtent() const {
        return m_extent;
    }

    VkSemaphore Swapchain::getImageAvailableSemaphore(std::uint32_t currentFrame) const {
        return m_imageAvailableSemaphores[currentFrame];
    }

    VkSemaphore Swapchain::getRenderFinishedSemaphore(std::uint32_t currentFrame) const {
        return m_renderFinishedSemaphores[currentFrame];
    }

    VkFence Swapchain::getInFlightFence(std::uint32_t currentFrame) const {
        return m_inFlightFences[currentFrame];
    }

    VkSwapchainKHR Swapchain::getSwapchain() const {
        return m_swapchain;
    }

    void Swapchain::createSwapchain() {
        Device::SwapchainSupportDetails swapchainSupportDetails = m_device.querySwapchainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupportDetails.formats);
        VkPresentModeKHR presentMode = choosePresentModes(swapchainSupportDetails.presentModes);
        VkExtent2D extent = chooseExtent(swapchainSupportDetails.capabilities);
    
        m_imageFormat = surfaceFormat.format;
        m_extent = extent;

        std::uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;

        if (swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapchainSupportDetails.capabilities.maxImageCount) {
            imageCount = swapchainSupportDetails.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.pNext = nullptr;
        swapchainCreateInfo.surface = m_device.getSurface();
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
        Device::QueueFamilyIndices queueFamilyIndices = m_device.findQueueFamilies();
        std::uint32_t indices[] = {
            queueFamilyIndices.graphicsFamilyIndex.value(),
            queueFamilyIndices.presentFamilyIndex.value()
        };

        if (queueFamilyIndices.graphicsFamilyIndex != queueFamilyIndices.presentFamilyIndex) {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = indices;
        } else {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0;
            swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        }

        swapchainCreateInfo.preTransform = swapchainSupportDetails.capabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = presentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_device.getDevice(), &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain.");
        }

        printPresentMode(presentMode);

        vkGetSwapchainImagesKHR(m_device.getDevice(), m_swapchain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device.getDevice(), m_swapchain, &imageCount, m_images.data());
    }

    void Swapchain::createImageViews() {
        m_imageViews.resize(m_images.size());

        for (std::size_t i = 0; i < m_images.size(); ++i) {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext = nullptr;
            imageViewCreateInfo.image = m_images[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = m_imageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device.getDevice(), &imageViewCreateInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views.");
            }
        }
    }

    void Swapchain::createRenderPass() {
        VkAttachmentDescription colorAttachmentDescription{};
        colorAttachmentDescription.format = m_imageFormat;
        colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.pNext = nullptr;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        if (vkCreateRenderPass(m_device.getDevice(), &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass.");
        }
    }

    void Swapchain::createFramebuffers() {
        m_framebuffers.resize(m_imageViews.size());

        for (std::size_t i = 0; i < m_imageViews.size(); ++i) {
            VkImageView attachments[] = {
                m_imageViews[i]
            };

            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.pNext = nullptr;
            framebufferCreateInfo.renderPass = m_renderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = attachments;
            framebufferCreateInfo.width = m_extent.width;
            framebufferCreateInfo.height = m_extent.height;
            framebufferCreateInfo.layers = 1;

            if (vkCreateFramebuffer(m_device.getDevice(), &framebufferCreateInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffers.");
            }
        }
    }

    void Swapchain::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            if (vkCreateSemaphore(m_device.getDevice(), &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_device.getDevice(), &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_device.getDevice(), &fenceCreateInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create sync objects.");
            }
        }
    }

    void Swapchain::cleanupSwapchain() {
        for (VkFramebuffer framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(m_device.getDevice(), framebuffer, nullptr);
        }

        for (VkImageView imageView : m_imageViews) {
            vkDestroyImageView(m_device.getDevice(), imageView, nullptr);
        }

        vkDestroySwapchainKHR(m_device.getDevice(), m_swapchain, nullptr);
    }

    VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const VkSurfaceFormatKHR &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR Swapchain::choosePresentModes(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    VkExtent2D Swapchain::chooseExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = m_windowExtent;

        actualExtent.width = std::clamp(
            actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );

        actualExtent.height = std::clamp(
            actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );

        return actualExtent;
    }

    void Swapchain::printPresentMode(const VkPresentModeKHR &presentMode) {
        std::string mode = "";
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            mode = "Mailbox";
        } else if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
            mode = "FIFO";
        } else if (presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            mode = "FIFO Relaxed";
        } else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            mode = "Immediate";
        }

        std::cout << "Present Mode: " << mode << '\n';
    }
}