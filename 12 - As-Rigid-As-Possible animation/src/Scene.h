#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Mesh.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/glut.h>
#endif

class Scene {
    // put here everything that you want
    std::vector< Mesh > meshes;

public:
    Scene() {}

    void addMesh(std::string const & modelFilename) {
        meshes.resize( meshes.size() + 1 );
        meshes[ meshes.size() - 1 ].loadOFF (modelFilename);
    }

    void draw() const {
        // iterer sur l'ensemble des objets, et faire leur rendu.
        for( unsigned int mIt = 0 ; mIt < meshes.size() ; ++mIt ) {
            Mesh const & mesh = meshes[mIt];
            mesh.draw();
        }
    }
};



#endif
