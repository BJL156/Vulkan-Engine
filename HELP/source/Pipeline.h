#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Swapchain.h"
#include "Model.h"

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

namespace eng {
	class Pipeline {
	public:
		Pipeline(Device &device, Swapchain &swapchain, const VkPipelineLayout &layout);
		~Pipeline();

		void bind(VkCommandBuffer commandBuffer);

		VkPipeline getPipeline() const;
	private:
		void createPipeline(const VkPipelineLayout &layout);

		VkShaderModule createShaderModule(const std::vector<char> shaderCode);
		static std::vector<char> readFile(const std::string &filename);

		VkPipeline m_pipeline;

		Device &m_device;
		Swapchain &m_swapchain;
	};
}

#endif