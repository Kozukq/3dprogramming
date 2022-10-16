#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>

#include <GL/glew.h>

#include <include/Camera.hpp>
#include <include/gameObject.hpp>

struct Scene {

	std::vector<struct Camera*> cameras;
	std::vector<struct GameObject*> gameObjects;
	ushort cameraID;

	inline Scene() : cameraID(0) {}

	inline void newCamera() { this->cameras.push_back(new Camera()); }
	
	inline void newGameObject() { this->gameObjects.push_back(new GameObject()); }

	inline void glDrawScene(GLuint programID, const struct Camera* camera) { for(struct GameObject* gameObject : this->gameObjects) gameObject->glDraw(programID,camera); }
};

#endif