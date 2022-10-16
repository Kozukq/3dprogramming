#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vector>

#include <glm/glm.hpp>

struct Texture {

	std::vector<unsigned char> rgb;
	unsigned int height;
	unsigned int width;

	inline Texture() {}

	void loadBMP(const char* filename);
};

#endif