#pragma once

#include <VULKAN/vulkan.h>
#include <GLM/glm.hpp>

#include "Device.hpp"

#include <array>
#include <vector>
#include <memory>
#include <stdexcept>

namespace ve {
	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
	};

	class Model {
	public:
		Model(Device& dev);
		~Model();

		void bind(VkCommandBuffer& commandBuffer);
		void draw(VkCommandBuffer& commandBuffer);
	private:
		void createModel();
		void createVertexBuffers();

		const std::vector<Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;

		Device& device;
	};
}