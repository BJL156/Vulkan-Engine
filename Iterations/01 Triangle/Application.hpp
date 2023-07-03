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

		VkCommandBuffer commandBuffer;

		const int MAX_FRAMES_IN_FLIGHT = 2;
	};
}