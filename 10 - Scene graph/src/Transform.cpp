#include <glm/gtc/matrix_transform.hpp>

#include <include/Transform.hpp>

glm::mat4 Transform::computeMatrix() {

    glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),glm::radians(this->rotation.x),glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),glm::radians(this->rotation.y),glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),glm::radians(this->rotation.z),glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 rotationMatrix = transformY * transformX * transformZ;

    return glm::translate(glm::mat4(1.0f),this->position) * rotationMatrix * glm::scale(glm::mat4(1.0f),this->scale);
}