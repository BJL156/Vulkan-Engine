#include "Application.h"

namespace eng {
	struct SimplePushConstantData {
		glm::mat2 transform{ 1.0f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	Application::Application() {
		loadGameObjects();
		createPipelineLayout();
	}

	Application::~Application() {
		vkDestroyPipelineLayout(m_device.getDevice(), m_pipelineLayout, nullptr);
	}

	void Application::run() {
		while (!m_window.shouldClose()) {
			m_window.update();

			drawFrame();
		}

		vkDeviceWaitIdle(m_device.getDevice());
	}

	void Application::loadGameObjects() {
		std::vector<Model::Vertex> vertices{
			{ {  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
			{ { -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } }
		};

		std::shared_ptr<Model> model = std::make_shared<Model>(m_device, vertices);

		GameObject triangle = GameObject::createGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2D.translation.x = 0.2f;
		triangle.transform2D.scale = { 2.0f, 0.5f };
		triangle.transform2D.rotation = 0.25f * glm::two_pi<float>();

		m_gameObjects.push_back(std::move(triangle));
	}

	void Application::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_device.getDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout.");
		}

		m_pipeline = std::make_unique<Pipeline>(m_device, m_renderer.getSwapchain(), m_pipelineLayout);
	}

	void Application::drawFrame() {
		VkCommandBuffer commandBuffer = m_renderer.beginFrame();
		m_renderer.beginSwapchainRenderPass(commandBuffer);

		renderGameObjects(commandBuffer);

		m_renderer.endSwapchainRenderPass(commandBuffer);
		m_renderer.endFrame();
	}

	void Application::renderGameObjects(VkCommandBuffer commandBuffer) {
		m_pipeline->bind(commandBuffer);

		for (GameObject &gameObject : m_gameObjects) {
			gameObject.transform2D.rotation = glm::mod(gameObject.transform2D.rotation + 0.001f, glm::two_pi<float>());

			SimplePushConstantData simplePushConstantData;
			simplePushConstantData.transform = gameObject.transform2D.getTransform();
			simplePushConstantData.offset = gameObject.transform2D.translation;
			simplePushConstantData.color = gameObject.color;

			vkCmdPushConstants(
				commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&simplePushConstantData
			);

			gameObject.model->bind(commandBuffer);
			gameObject.model->draw(commandBuffer);
		}
	}
}