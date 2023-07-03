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
		void createVertexBuffer();
		void createIndexBuffer();

		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f},	{1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f},		{0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f},		{1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f},		{0.0f, 0.0f, 1.0f}}
		};
		const std::vector<uint16_t> indices = {
			2, 0, 3,
			1, 2, 3
		};

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		Device& device;
	};
}