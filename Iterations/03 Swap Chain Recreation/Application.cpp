#include "Application.hpp"

namespace ve {
	Application::Application() {
		createCommandBuffers();
	}

	Application::~Application() {

	}

	void Application::run() {
		while (!window.shouldClose()) {
			window.pollEvents();

			drawFrame();
		}

		vkDeviceWaitIdle(device.getDevice());
	}

	void Application::drawFrame() {
		uint32_t imageIndex;

		VkResult result = swapChain.acquireNextImage(imageIndex, currentFrame);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			swapChain.recreateSwapChain();
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire next image from the swap chain.");
		}

		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		result = swapChain.submitCommandBuffers(commandBuffers[currentFrame], imageIndex, currentFrame);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			swapChain.recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit the command buffers.");
		}

		currentFrame = (currentFrame + 1) % swapChain.getMaxFramesInFlight();
	}

	void Application::createCommandBuffers() {
		commandBuffers.resize(swapChain.getMaxFramesInFlight());

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = device.getCommandPool();
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.getDevice(), &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate the command buffers.");
		}
	}

	void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording the command buffers.");
		}

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = swapChain.getRenderPass();
		renderPassBeginInfo.framebuffer = swapChain.getFramebuffers()[imageIndex];
		renderPassBeginInfo.renderArea.offset = {0, 0};
		renderPassBeginInfo.renderArea.extent = swapChain.getExtent();
		VkClearValue clearColor = {{{0.01f, 0.01f, 0.01f, 1.0f}}};
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getGraphicsPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain.getExtent().width);
		viewport.height = static_cast<float>(swapChain.getExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapChain.getExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record the command buffer.");
		}

	}
}