#pragma once

#include <VULKAN/vulkan.h>

#include "Device.hpp"
#include "SwapChain.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

namespace ve {
	class Pipeline {
	public:
		Pipeline(Device& dev, SwapChain& swap);
		~Pipeline();

		VkPipeline getGraphicsPipeline() { return graphicsPipeline; }
	private:
		void createPipeline();
		void createGraphicsPipeline();

		static std::vector<char> getFileContents(const std::string& filepath);
		VkShaderModule createShaderModule(std::vector<char>& shaderCode);

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		Device& device;
		SwapChain& swapChain;
	};
}