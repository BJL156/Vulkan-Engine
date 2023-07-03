#pragma once

#include "Window.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Pipeline.hpp"

namespace ve {
	class Application {
	public:
		Application();
		~Application();

		void run();
	private:
		void drawFrame();
		void createCommandBuffers();
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		Window window{800, 600, "Orange Juice Yucky"};
		Device device{window};
		SwapChain swapChain{device, window};
		Pipeline pipeline{device, swapChain};

		uint32_t currentFrame = 0;

		std::vector<VkCommandBuffer> commandBuffers;
	};
}