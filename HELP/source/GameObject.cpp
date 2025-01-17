#include "GameObject.h"

namespace eng {
    GameObject GameObject::createGameObject() {
        static unsigned int currentId = 0;
        return GameObject{ currentId++ };
    }

    unsigned int GameObject::getId() const {
        return m_id;
    }

    GameObject::GameObject(unsigned int id)
        : m_id(id) {
    }

    glm::mat4 TransformComponent::getTransform() {
        glm::mat4 transform = glm::translate({ 1.0f }, translation);

        transform = glm::rotate(transform, rotation.y, { 0.0f, 1.0f, 0.0f });
        transform = glm::rotate(transform, rotation.x, { 1.0f, 0.0f, 0.0f });
        transform = glm::rotate(transform, rotation.z, { 0.0f, 0.0f, 1.0f });

        transform = glm::scale(transform, scale);

        return transform;
    }
}