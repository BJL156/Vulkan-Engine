#include "Application.h"

namespace eng {
	struct TransformPushConstantData {
		glm::mat4 transform{ 1.0f };
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
			{ { -.5f, -.5f, -.5f }, {.9f, .9f, .9f } },
			{ { -.5f, .5f, .5f }, {.9f, .9f, .9f } },
			{ { -.5f, -.5f, .5f }, {.9f, .9f, .9f } },
			{ { -.5f, -.5f, -.5f }, {.9f, .9f, .9f } },
			{ { -.5f, .5f, -.5f }, {.9f, .9f, .9f } },
			{ { -.5f, .5f, .5f }, {.9f, .9f, .9f } },

			{ { .5f, -.5f, -.5f }, {.8f, .8f, .1f } },
			{ { .5f, .5f, .5f }, {.8f, .8f, .1f } },
			{ { .5f, -.5f, .5f }, {.8f, .8f, .1f } },
			{ { .5f, -.5f, -.5f }, {.8f, .8f, .1f } },
			{ { .5f, .5f, -.5f }, {.8f, .8f, .1f } },
			{ { .5f, .5f, .5f }, {.8f, .8f, .1f } },

			{ { -.5f, -.5f, -.5f }, {.9f, .6f, .1f } },
			{ { .5f, -.5f, .5f }, {.9f, .6f, .1f } },
			{ { -.5f, -.5f, .5f }, {.9f, .6f, .1f } },
			{ { -.5f, -.5f, -.5f }, {.9f, .6f, .1f } },
			{ { .5f, -.5f, -.5f }, {.9f, .6f, .1f } },
			{ { .5f, -.5f, .5f }, {.9f, .6f, .1f } },

			{ { -.5f, .5f, -.5f }, {.8f, .1f, .1f } },
			{ { .5f, .5f, .5f }, {.8f, .1f, .1f } },
			{ { -.5f, .5f, .5f }, {.8f, .1f, .1f } },
			{ { -.5f, .5f, -.5f }, {.8f, .1f, .1f } },
			{ { .5f, .5f, -.5f }, {.8f, .1f, .1f } },
			{ { .5f, .5f, .5f }, {.8f, .1f, .1f } },

			{ { -.5f, -.5f, 0.5f }, {.1f, .1f, .8f} },
			{ { .5f, .5f, 0.5f }, {.1f, .1f, .8f} },
			{ { -.5f, .5f, 0.5f }, {.1f, .1f, .8f} },
			{ { -.5f, -.5f, 0.5f }, {.1f, .1f, .8f} },
			{ { .5f, -.5f, 0.5f }, {.1f, .1f, .8f} },
			{ { .5f, .5f, 0.5f }, {.1f, .1f, .8f }},

			{ { -.5f, -.5f, -0.5f }, {.1f, .8f, .1f} },
			{ { .5f, .5f, -0.5f }, {.1f, .8f, .1f} },
			{ { -.5f, .5f, -0.5f }, {.1f, .8f, .1f} },
			{ { -.5f, -.5f, -0.5f }, {.1f, .8f, .1f} },
			{ { .5f, -.5f, -0.5f }, {.1f, .8f, .1f} },
			{ { .5f, .5f, -0.5f }, {.1f, .8f, .1f }}
		};

		std::shared_ptr<Model> model = std::make_shared<Model>(m_device, vertices);

		GameObject triangle = GameObject::createGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform.translation = { 0.0f, 0.0f, 0.5f };
		triangle.transform.scale = { 0.5f, 0.5f, 0.5f };
		triangle.transform.rotation = { 0.0f, 0.0f, 0.0f};

		m_gameObjects.push_back(std::move(triangle));
	}

	void Application::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(TransformPushConstantData);

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
			gameObject.transform.rotation.x = glm::mod(gameObject.transform.rotation.x + 0.001f, glm::two_pi<float>());
			gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y + 0.001f, glm::two_pi<float>());

			TransformPushConstantData transformPushConstantData;
			transformPushConstantData.transform = gameObject.transform.getTransform();

			vkCmdPushConstants(
				commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(TransformPushConstantData),
				&transformPushConstantData
			);

			gameObject.model->bind(commandBuffer);
			gameObject.model->draw(commandBuffer);
		}
	}
}