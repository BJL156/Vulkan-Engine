#ifndef MODEL_H
#define MODEL_H

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Device.h"

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstring>

namespace eng {
	class Model {
	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		Model(Device &device, const std::vector<Vertex> &vertices);
		~Model();

		Model(const Model &) = delete;
		void operator=(const Model &) = delete;
		Model(Model &&) = delete;
		Model &operator=(Model &&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);

		Device &m_device;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		std::uint32_t m_vertexCount;
	};
}

#endif