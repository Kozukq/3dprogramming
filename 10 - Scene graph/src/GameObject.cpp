#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <include/GameObject.hpp>
#include <include/Camera.hpp>

void GameObject::glDraw(GLuint programID, const struct Camera* camera) {

    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint uvBuffer;

    GLuint verticesID;
    GLuint textureID;
    
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 MVP;

    // Chargement du buffer de sommets
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.vertices.size() * sizeof(glm::vec3), &this->mesh.vertices[0], GL_STATIC_DRAW);

    // Chargement du buffer d'indices de sommets pour former les triangles
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->mesh.triangles.size() * sizeof(unsigned int), &this->mesh.triangles[0], GL_STATIC_DRAW);

    // Chargement du buffer de coordonnées de textures
    glGenBuffers(1,&uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,uvBuffer);
    glBufferData(GL_ARRAY_BUFFER,this->mesh.uv.size() * sizeof(glm::vec2),&this->mesh.uv[0],GL_STATIC_DRAW);

    // ???
    glGenVertexArrays(1,&verticesID);
    glBindVertexArray(verticesID);

    // Envoi des buffers au GPU
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elementBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,uvBuffer);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);

    // Chargement et envoi de la texture au GPU
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_2D,textureID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,this->texture.width,this->texture.height,0,GL_BGR,GL_UNSIGNED_BYTE,this->texture.rgb.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Calcul et envoi de la Matrice MVP
    modelMatrix = glm::mat4(1.0f);
    viewMatrix = glm::lookAt(camera->position,camera->position+camera->target,camera->up);
    projectionMatrix = glm::perspective(glm::radians(camera->fov),camera->aspectRatio,camera->nearPlane,camera->farPlane);
    MVP = projectionMatrix * viewMatrix * modelMatrix * this->transform.matrix;
    glUniformMatrix4fv(glGetUniformLocation(programID,"MVP"),1,false,&MVP[0][0]);

    this->update();

    // Dessine le GameObject
    glDrawElements(GL_TRIANGLES,this->mesh.triangles.size(),GL_UNSIGNED_INT,(void*)0);

    // Nettoyage
    glDeleteBuffers(1,&vertexBuffer);
    glDeleteBuffers(1,&elementBuffer);
    glDeleteBuffers(1,&uvBuffer);
    glDeleteVertexArrays(1,&verticesID);
    glDeleteTextures(1,&textureID);

    for(struct GameObject* child : this->children) child->glDraw(programID,camera);
}

void GameObject::update() {

	if(this->parent) {

		this->transform.matrix = this->parent->transform.matrix * this->transform.computeMatrix();
	}

	else {

		this->transform.matrix = this->transform.computeMatrix();
	}
}