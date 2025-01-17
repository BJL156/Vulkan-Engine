#ifndef APPLICATION_H
#define APPLICATION_H

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Window.h"
#include "Device.h"
#include "Pipeline.h"
#include "Model.h"
#include "GameObject.h"
#include "Renderer.h"

#include <vector>
#include <stdexcept>
#include <memory>

namespace eng {
	class Application {
	public:
		Application();
		~Application();

		void run();
	private:
		void loadGameObjects();
		void createPipelineLayout();

		void drawFrame();
		void renderGameObjects(VkCommandBuffer commandBuffer);

		VkPipelineLayout m_pipelineLayout;

		Window m_window{ 800, 600, "Vulkan Engine" };
		Device m_device{ m_window };
		std::unique_ptr<Pipeline> m_pipeline;
		std::vector<GameObject> m_gameObjects;

		Renderer m_renderer{ m_window, m_device };
	};
}

#endif