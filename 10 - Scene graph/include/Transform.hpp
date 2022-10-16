#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>

struct Transform {

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 matrix;

	inline Transform() : position(glm::vec3(0.0f,0.0f,0.0f)), rotation(glm::vec3(1.0f,1.0f,1.0f)), scale(glm::vec3(1.0f,1.0f,1.0f)), matrix(computeMatrix()) {}

	glm::mat4 computeMatrix();
};

#endif