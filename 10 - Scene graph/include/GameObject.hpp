#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include <iostream>
#include <vector>
#include <string>

#include <GL/glew.h>

#include <include/Transform.hpp>
#include <include/Mesh.hpp>
#include <include/Texture.hpp>
#include <include/Camera.hpp>

struct GameObject {

	struct GameObject* parent;
	std::vector<struct GameObject*> children;
	struct Transform transform;
	struct Mesh mesh;
	struct Texture texture;
	std::string tag;

	inline GameObject() : parent(nullptr), transform(Transform()), mesh(Mesh()), texture(Texture()) {}
	inline GameObject(struct GameObject* _parent) : parent(_parent), transform(Transform()), mesh(Mesh()), texture(Texture()) {}

	inline void newChild() { this->children.push_back(new GameObject(this)); }

	inline void addMesh(const std::string& filename) { this->mesh.loadOFF(filename); this->mesh.computeNormals(); }
	inline void addSphereMesh(unsigned int nbMeridians) { this->mesh.generateSphere(nbMeridians); this->mesh.computeNormals(); }

	inline void addTexture(const char* filename) { this->texture.loadBMP(filename); }

	inline void addTag(const std::string& tag) { this->tag = tag; }

	friend std::ostream& operator<<(std::ostream& ostream, const struct GameObject* gameObject) {

		std::string tag = gameObject->tag.empty() ? "undefined" : gameObject->tag;

		return ostream << tag << ": " << gameObject->transform.position.x << " " << gameObject->transform.position.y << " " << gameObject->transform.position.z;
	}

	void glDraw(GLuint programID, const struct Camera* camera);
	void update();
};

#endif