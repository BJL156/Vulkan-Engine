#include "Renderer.h"

namespace eng {
	Renderer::Renderer(Window &window, Device &device)
		: m_window(window), m_device(device) {
		createCommandBuffers();
	}

	Renderer::~Renderer() {
		freeCommandBuffers();
	}

	VkCommandBuffer Renderer::beginFrame() {
		const VkFence inFlightFence = m_swapchain.getInFlightFence(m_currentFrame);

		vkWaitForFences(m_device.getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(m_device.getDevice(), 1, &inFlightFence);

		VkResult result = vkAcquireNextImageKHR(m_device.getDevice(), m_swapchain.getSwapchain(), UINT64_MAX, m_swapchain.getImageAvailableSemaphore(m_currentFrame), VK_NULL_HANDLE, &m_imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_swapchain.recreateSwapchain();
			return nullptr;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire next swapchain image.");
		}

		VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin commander buffer.");
		}

		return commandBuffer;
	}

	void Renderer::endFrame() {
		if (vkEndCommandBuffer(m_commandBuffers[m_currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer.");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;

		VkSemaphore waitSemaphores[] = { m_swapchain.getImageAvailableSemaphore(m_currentFrame) };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

		VkSemaphore signalSemaphores[] = { m_swapchain.getRenderFinishedSemaphore(m_currentFrame) };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, m_swapchain.getInFlightFence(m_currentFrame)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer.");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { m_swapchain.getSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &m_imageIndex;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.getResizeFlag()) {
			while (m_window.getWidth() == 0 || m_window.getHeight() == 0) {
				glfwWaitEvents();
			}

			m_window.resetResizeFlag();
			m_swapchain.recreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swapchain image.");
		}

		m_currentFrame = (m_currentFrame + 1) % m_swapchain.MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::beginSwapchainRenderPass(VkCommandBuffer commandBuffer) {
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_swapchain.getRenderPass();
		renderPassBeginInfo.framebuffer = m_swapchain.getFramebuffer(m_imageIndex);
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = m_swapchain.getExtent();

		VkClearValue clearColor = { { { 0.01f, 0.01f, 0.01f } } };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapchain.getExtent().width);
		viewport.height = static_cast<float>(m_swapchain.getExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapchain.getExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endSwapchainRenderPass(VkCommandBuffer commandBuffer) {
		vkCmdEndRenderPass(commandBuffer);
	}

	Swapchain& Renderer::getSwapchain() {
		return m_swapchain;
	}

	void Renderer::createCommandBuffers() {
		m_commandBuffers.resize(m_swapchain.MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = m_device.getCommandPool();
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = static_cast<std::uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.getDevice(), &commandBufferAllocateInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer.");
		}
	}
	
	void Renderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			m_device.getDevice(),
			m_device.getCommandPool(),
			static_cast<std::uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data()
		);

		m_commandBuffers.clear();
	}
}