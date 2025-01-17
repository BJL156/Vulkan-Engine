#include "Model.h"

namespace eng {
	Model::Model(Device &device, const std::vector<Vertex> &vertices)
		: m_device(device) {
		createVertexBuffers(vertices);
	}

	Model::~Model() {
		vkDestroyBuffer(m_device.getDevice(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_device.getDevice(), m_vertexBufferMemory, nullptr);
	}

	void Model::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void Model::draw(VkCommandBuffer commandBuffer) {
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}

	void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
		m_vertexCount = static_cast<std::uint32_t>(vertices.size());
		if (m_vertexCount < 3) {
			throw std::runtime_error("Model vertex count must be at least 3.");
		}

		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_vertexBuffer,
			m_vertexBufferMemory
		);

		void *data;
		vkMapMemory(m_device.getDevice(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<std::size_t>(bufferSize));
		vkUnmapMemory(m_device.getDevice(), m_vertexBufferMemory);
	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindDescriptions() {
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions(1);
		vertexInputBindingDescriptions[0].binding = 0;
		vertexInputBindingDescriptions[0].stride = sizeof(Vertex);
		vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(2);
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, position);

		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

		return vertexInputAttributeDescriptions;
	}
}