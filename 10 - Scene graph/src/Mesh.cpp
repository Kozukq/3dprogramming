#include <iostream>
#include <fstream>

#include <glm/glm.hpp>

#include <include/Mesh.hpp>

void Mesh::generateSphere(unsigned int nbMeridians) {

    float x, y, z, u, v;
    float theta, phi;

    this->vertices.clear();
    this->triangles.clear();
    this->uv.clear();

    for(unsigned int i = 0; i < nbMeridians; i++) {

        u = (float)i / (float)(nbMeridians-1);

        theta = 2 * M_PI * u;

        for(unsigned int j = 0; j < nbMeridians; j++) {
            
            v = (float)j / (float)(nbMeridians-1);
            
            phi = -M_PI / 2.0f + v * M_PI;

            x = cos(theta) * cos(phi);
            y = sin(theta) * cos(phi);
            z = sin(phi);

            this->vertices.push_back(glm::vec3(x,y,z));
            this->uv.push_back(glm::vec2(u,v));

            if(i < nbMeridians-1 && j < nbMeridians-1) {

                this->triangles.push_back(i+j*nbMeridians);
                this->triangles.push_back((i+1)+j*nbMeridians);
                this->triangles.push_back((i+1)+(j+1)*nbMeridians);

                this->triangles.push_back(i+j*nbMeridians);
                this->triangles.push_back((i+1)+(j+1)*nbMeridians);
                this->triangles.push_back(i+(j+1)*nbMeridians);
            }
        }
    }
}

void Mesh::loadOFF(const std::string& filename) {
    
    std::string buffer;
    unsigned int vSize, tSize, fSize, nbVertices;

    this->vertices.clear(); 
    this->triangles.clear();
    this->uv.clear();

    std::cout << "loadOFF : " << filename << std::endl;
    
    std::ifstream stream(filename.c_str());
    if(!stream) {
        
        std::cout << "ERROR: can't open mesh at " << filename << std::endl;
        stream.close();
        exit(EXIT_FAILURE);
    }

    stream >> buffer >> vSize >> tSize >> fSize;

    this->vertices.resize(vSize);
    this->triangles.resize(tSize * 3);

    for(unsigned int i = 0; i < vSize; i++) { 
        
        float x, y, z;

        stream >> x >> y >> z;

        this->vertices[i] = glm::vec3(x,y,z);
    }

    for(unsigned int i = 0; i < tSize * 3; i += 3) stream >> nbVertices >> this->triangles[i] >> this->triangles[i+1] >> this->triangles[i+2];

    // Calcul audacieux des UV
    for(unsigned int i = 0; i < (unsigned int)glm::sqrt(vSize); i++) {

        float u = (float)i / (float)(glm::sqrt(vSize)-1);

        for(unsigned int j = 0; j < (unsigned int)glm::sqrt(vSize); j++) {

            float v = (float)j / (float)(glm::sqrt(vSize)-1);

            this->uv.push_back(glm::vec2(u,v));
        }
    }

    stream.close();
}

void Mesh::computeNormals() {

    unsigned int vSize, tSize;

    this->normals.clear();

    vSize = this->vertices.size();
    tSize = this->triangles.size();

    for(unsigned int i = 0; i < tSize; i += 3) {

        glm::vec3 v1 = glm::vec3(this->vertices[this->triangles[i]].x,this->vertices[this->triangles[i]].y,this->vertices[this->triangles[i]].z);
        glm::vec3 v2 = glm::vec3(this->vertices[this->triangles[i+1]].x,this->vertices[this->triangles[i+1]].y,this->vertices[this->triangles[i+1]].z);
        glm::vec3 v3 = glm::vec3(this->vertices[this->triangles[i+2]].x,this->vertices[this->triangles[i+2]].y,this->vertices[this->triangles[i+2]].z);

        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;

        glm::vec3 normal = glm::cross(edge1,edge2);

        normal = glm::normalize(normal);

        this->normals.push_back(normal);
    }
}