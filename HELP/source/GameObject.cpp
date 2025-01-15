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

    glm::mat2 Transform2DComponent::getTransform() {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotationMatrix{ { c, s }, { -s, c } };

        glm::mat2 scaleMatrix{{ scale.x, 0.0f }, { 0.0f, scale.y }};

        return rotationMatrix * scaleMatrix;
    }
}