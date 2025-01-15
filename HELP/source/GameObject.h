#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Model.h"

#include <memory>

namespace eng {
	struct Transform2DComponent {
		glm::vec2 translation = { 0.0f, 0.0f };
		glm::vec2 scale{ 1.0f, 1.0f };
		float rotation = 0.0f;

		glm::mat2 getTransform();
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
		Transform2DComponent transform2D{};
	private:
		GameObject(unsigned int id);

		unsigned int m_id;
	};
}

#endif