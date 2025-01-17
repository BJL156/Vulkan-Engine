#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"

#include <memory>

namespace eng {
	struct TransformComponent {
		glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };

		glm::mat4 getTransform();
	};

	class GameObject {
	public:
		GameObject(const GameObject &) = delete;
		GameObject &operator=(const GameObject &) = delete;
		GameObject(GameObject &&) = default;
		GameObject &operator=(GameObject &&) = default;

		static GameObject createGameObject();

		unsigned int getId() const;

		std::shared_ptr<Model> model{};
		glm::vec3 color{};
		TransformComponent transform{};
	private:
		GameObject(unsigned int id);

		unsigned int m_id;
	};
}

#endif