#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

struct Camera {

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	float yaw;
	float pitch;

	float fov;
	float aspectRatio;
	float nearPlane;
	float farPlane;

	inline Camera() {

		this->position = glm::vec3(0.0f,0.0f,0.0f);
		this->target = glm::vec3(0.0f,0.0f,0.0f);
		this->up = glm::vec3(0.0f,1.0f,0.0f);

		this->yaw = 0.0f;
		this->pitch = 0.0f;

		this->fov = 45.0f;
		this->aspectRatio = 4.0f/3.0f;
		this->nearPlane = 0.1f;
		this->farPlane = 150.0f;
	}
};

#endif