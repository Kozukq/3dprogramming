#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <string>

#include <glm/glm.hpp>

struct Mesh {

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> triangles;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;

    inline Mesh() {};

	void generateSphere(unsigned int nbMeridians);
    void loadOFF(const std::string& filename);
    void computeNormals();
};

#endif