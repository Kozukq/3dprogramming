// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unistd.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>

void processInput(GLFWwindow *window);
void generateSurface(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::vector<float>& uv, int length, float delta);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
#define FREE_MODE 0
#define ORBITAL_MODE 1

int camera_mode = FREE_MODE;
glm::vec3 free_camera_position = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 free_camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 free_camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 orbital_camera_position = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 orbital_camera_target = glm::vec3(-0.5f, -0.5f, 0.0f);
glm::vec3 orbital_camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// rotations de la caméra libre
float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;

// rotation de la surface générée
float surfaceRotation = 0.0f;
float inputRotation = 0.01f;

// buffer d'input clavier
bool isPressed = false;

/*******************************************************************************/

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP2 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    //  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    /*****************TODO***********************/
    // Get a handle for our "Model View Projection" matrices uniforms
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 MVP;

    /****************************************/
    // Définition de la scène
    std::vector<unsigned short> indices_surface;
    std::vector<glm::vec3> indexed_vertices_surface;
    std::vector<float> uv_surface;
    int resolution = 16;
    float delta = 0.05f;
    generateSurface(indices_surface,indexed_vertices_surface,uv_surface,resolution,delta);
    GLuint vertexbuffer;
    GLuint elementbuffer;

    // Gestion de la heightmap
    GLuint hmap = loadBMP_custom("hmaps/hmap_cliffs.bmp");
    GLuint hmapID = glGetUniformLocation(programID,"hmapSampler");

    // Gestion des textures
    GLuint TextureGrass = loadBMP_custom("textures/tex_grass.bmp");
    GLuint TextureRock = loadBMP_custom("textures/tex_rock.bmp");
    GLuint TextureSnowrock = loadBMP_custom("textures/tex_snowrock.bmp");
    GLuint TextureGrassID = glGetUniformLocation(programID,"textureSamplerGrass");
    GLuint TextureRockID = glGetUniformLocation(programID,"textureSamplerRock");
    GLuint TextureSnowrockID = glGetUniformLocation(programID,"textureSamplerSnowrock");
    GLuint uvbuffer;
    glGenBuffers(1,&uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER,uvbuffer);
    glBufferData(GL_ARRAY_BUFFER,uv_surface.size()*sizeof(float),&uv_surface[0],GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{

        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // Load vertex buffer
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_vertices_surface.size() * sizeof(glm::vec3), &indexed_vertices_surface[0], GL_STATIC_DRAW);

        // Generate a buffer for the indices as well
        glGenBuffers(1, &elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_surface.size() * sizeof(unsigned short), &indices_surface[0] , GL_STATIC_DRAW);

        // Paramétrage des transformations en fonction du mode de caméra
        switch(camera_mode) {

            case FREE_MODE: {

                modelMatrix = glm::mat4(1.0f);

                viewMatrix = glm::lookAt(free_camera_position,free_camera_position+free_camera_target,free_camera_up);
                viewMatrix = glm::rotate(viewMatrix,yaw,glm::vec3(1.0f,0.0f,0.0f));
                viewMatrix = glm::rotate(viewMatrix,pitch,glm::vec3(0.0f,1.0f,0.0f));
                viewMatrix = glm::rotate(viewMatrix,roll,glm::vec3(0.0f,0.0f,1.0f));

                projectionMatrix = glm::perspective(glm::radians(45.0f),4.0f/3.0f,0.1f,100.0f);

                break;
            }

            case ORBITAL_MODE: {

                surfaceRotation += inputRotation;

                modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix,glm::vec3(-resolution*delta,-2*resolution*delta,0.0f));
                modelMatrix = glm::translate(modelMatrix,glm::vec3(resolution*delta/2,resolution*delta/2,0.0f));
                modelMatrix = glm::rotate(modelMatrix,surfaceRotation,glm::vec3(0.0f,0.0f,1.0f));
                modelMatrix = glm::translate(modelMatrix,glm::vec3(-resolution*delta/2,-resolution*delta/2,0.0f));

                viewMatrix = glm::lookAt(orbital_camera_position,orbital_camera_target,orbital_camera_up);
                viewMatrix = glm::rotate(viewMatrix,225.0f,glm::vec3(1.0f,0.0f,0.0f));

                projectionMatrix = glm::perspective(glm::radians(45.0f),4.0f/3.0f,0.1f,100.0f);

                break;
            }
        }

        // Matrice MVP
        MVP = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(programID,"MVP"),1,false,&MVP[0][0]);

        // Sommets et indices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Height map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,hmap);
        glUniform1i(hmapID,0);

        // Textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,TextureGrass);
        glUniform1i(TextureGrassID,1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,TextureRock);
        glUniform1i(TextureRockID,2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D,TextureSnowrock);
        glUniform1i(TextureSnowrockID,3);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER,uvbuffer);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices_surface.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    switch(camera_mode) {

        case FREE_MODE: {

            // Camera zoom in and out
            float cameraSpeed = 2.5 * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) free_camera_position += cameraSpeed * free_camera_target;
            if (glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS) free_camera_position -= cameraSpeed * free_camera_target;

            // Translations
            if(glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS) free_camera_position += free_camera_up * cameraSpeed;
            if(glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS) free_camera_position -= free_camera_up * cameraSpeed;
            if(glfwGetKey(window,GLFW_KEY_LEFT) == GLFW_PRESS) free_camera_position -= glm::normalize(glm::cross(free_camera_target,free_camera_up)) * cameraSpeed;
            if(glfwGetKey(window,GLFW_KEY_RIGHT) == GLFW_PRESS) free_camera_position += glm::normalize(glm::cross(free_camera_target,free_camera_up)) * cameraSpeed;

            // Rotation X
            if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) yaw += 0.1f;
            if(glfwGetKey(window,GLFW_KEY_Q) == GLFW_PRESS) yaw -= 0.1f;

            // Rotation Y
            if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) pitch += 0.1f;
            if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) pitch -= 0.1f;

            // Rotation Z
            if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) roll += 0.1f;
            if(glfwGetKey(window,GLFW_KEY_E) == GLFW_PRESS) roll -= 0.1f;

            break;
        }

        case ORBITAL_MODE: {

            if(glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS) inputRotation += 0.001f;
            if(glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS) inputRotation -= 0.001f;

            break;
        }
    }

    // Changement de caméra
    if(!isPressed && glfwGetKey(window,GLFW_KEY_C) == GLFW_PRESS) { 
        
        isPressed = true;
        camera_mode = camera_mode == FREE_MODE ? ORBITAL_MODE : FREE_MODE;
    }
    if(glfwGetKey(window,GLFW_KEY_C) == GLFW_RELEASE) isPressed = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void generateSurface(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::vector<float>& uv, int length, float delta) {

    indexed_vertices.resize(length*length);

    for(int i = 0; i < length; i++) {

        for(int j = 0; j < length; j++) {

            indexed_vertices[i*length+j] = glm::vec3((float)j*delta,(float)i*delta,0.0f);
            uv.push_back((float)j/(length-1));
            uv.push_back((float)i/(length-1));
        }
    }

    for(int i = 0; i < length-1; i++) {

        for(int j = 0; j < length-1; j++) {

            indices.push_back(i*length+j);
            indices.push_back(i*length+j+length);
            indices.push_back(i*length+j+1);

            indices.push_back(i*length+j+1);
            indices.push_back(i*length+j+length);
            indices.push_back(i*length+j+length+1);
        }
    }
}