#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <include/Scene.hpp>
#include <include/shader.hpp>

#define GLOBAL_SPEED 1

GLFWwindow* glSetup();
void processInput(GLFWwindow* window);

struct Scene scene;
float deltaTime = 0.0f;
bool isPressed = false;

int main(void) {

    GLFWwindow* window;
    GLuint programID;
    float lastFrame;
    float currentFrame;

    window = glSetup();

    scene = Scene();

    scene.newCamera();
    scene.cameras[0]->position = glm::vec3(0.0f, 0.0f, 50.0f);
    scene.cameras[0]->yaw = -90.0f;

    scene.newCamera();
    scene.cameras[1]->position = glm::vec3(0.0f, 0.0f, -50.0f);
    scene.cameras[1]->yaw = 90.0f;

    scene.newCamera();
    scene.cameras[2]->position = glm::vec3(40.0f, 40.0f, 0.0f);
    scene.cameras[2]->fov = 60.0f;
    scene.cameras[2]->yaw = 180.0f;
    scene.cameras[2]->pitch = -45.0f;
    
    scene.newGameObject();
    scene.gameObjects[0]->addTag("Sun");
    scene.gameObjects[0]->addSphereMesh(100);
    scene.gameObjects[0]->addTexture("../res/texture/1904_sun.bmp");
    scene.gameObjects[0]->transform.scale = glm::vec3(5.0f,5.0f,5.0f);

    scene.gameObjects[0]->newChild();
    scene.gameObjects[0]->children[0]->addTag("Earth");
    scene.gameObjects[0]->children[0]->addSphereMesh(100);
    scene.gameObjects[0]->children[0]->addTexture("../res/texture/1904_earth.bmp");
    scene.gameObjects[0]->children[0]->transform.scale = glm::vec3(0.25f,0.25f,0.25f);
    scene.gameObjects[0]->children[0]->transform.rotation = glm::vec3(-90.0f+23.44f,0.0f,0.0f);
    scene.gameObjects[0]->children[0]->transform.position = glm::vec3(5.0f,0.0f,0.0f);

    scene.gameObjects[0]->children[0]->newChild();
    scene.gameObjects[0]->children[0]->children[0]->addTag("Moon");
    scene.gameObjects[0]->children[0]->children[0]->addSphereMesh(100);
    scene.gameObjects[0]->children[0]->children[0]->addTexture("../res/texture/2048_moon.bmp");
    scene.gameObjects[0]->children[0]->children[0]->transform.scale = glm::vec3(0.25f,0.25f,0.25f);
    scene.gameObjects[0]->children[0]->children[0]->transform.position = glm::vec3(2.0f,0.0f,0.0f);

    scene.newGameObject();
    scene.gameObjects[1]->addTag("Background");
    scene.gameObjects[1]->addSphereMesh(100);
    scene.gameObjects[1]->addTexture("../res/texture/1904_milkyway.bmp");
    scene.gameObjects[1]->transform.scale = glm::vec3(75.0f,75.0f,75.0f);

    programID = LoadShaders("../res/shaders/vertex_shader.glsl","../res/shaders/fragment_shader.glsl");
    glUseProgram(programID);

    lastFrame = glfwGetTime();

    while(glfwGetKey(window,GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) {
 
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.gameObjects[0]->transform.rotation += glm::vec3(0.0f,10*deltaTime*GLOBAL_SPEED,0.0f);
        scene.gameObjects[0]->children[0]->transform.rotation += glm::vec3(0.0f,100*deltaTime*GLOBAL_SPEED,0.0f);
        scene.gameObjects[0]->children[0]->children[0]->transform.rotation += glm::vec3(0.0f,100*deltaTime*GLOBAL_SPEED,0.0f);
        scene.gameObjects[1]->transform.rotation += glm::vec3(0.0f,5*deltaTime,0.0f);
        scene.glDrawScene(programID,scene.cameras[scene.cameraID]);

        glfwSwapBuffers(window);
        glfwPollEvents();   
    }

    glDeleteProgram(programID);
    glfwTerminate();

    return 0;
}

GLFWwindow* glSetup() {

    GLFWwindow* window;

    if(!glfwInit()) {
        
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow( 1024, 768, "TP3 - GLFW", NULL, NULL);
    if(window == NULL) {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if(glewInit() != GLEW_OK){

        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return window;
}

void processInput(GLFWwindow* window) {

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    glm::vec3 direction;
    float cameraSpeed = deltaTime;

    // Déplacements ZQSD
    if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) scene.cameras[scene.cameraID]->position += 25.0f * cameraSpeed * scene.cameras[scene.cameraID]->target;
    if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) scene.cameras[scene.cameraID]->position -= 25.0f * cameraSpeed * scene.cameras[scene.cameraID]->target;
    if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) scene.cameras[scene.cameraID]->position -= glm::normalize(glm::cross(scene.cameras[scene.cameraID]->target,scene.cameras[scene.cameraID]->up)) * 20.0f * cameraSpeed;
    if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) scene.cameras[scene.cameraID]->position += glm::normalize(glm::cross(scene.cameras[scene.cameraID]->target,scene.cameras[scene.cameraID]->up)) * 20.0f * cameraSpeed;

    // Rotations HBGD
    if(glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS) scene.cameras[scene.cameraID]->pitch += 2.0f;
    if(glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS) scene.cameras[scene.cameraID]->pitch -= 2.0f;
    if(glfwGetKey(window,GLFW_KEY_LEFT) == GLFW_PRESS) scene.cameras[scene.cameraID]->yaw -= 2.0f;
    if(glfwGetKey(window,GLFW_KEY_RIGHT) == GLFW_PRESS) scene.cameras[scene.cameraID]->yaw += 2.0f;

    if(scene.cameras[scene.cameraID]->pitch > 89.0f) scene.cameras[scene.cameraID]->pitch = 89.0f;
    if(scene.cameras[scene.cameraID]->pitch < -89.0f) scene.cameras[scene.cameraID]->pitch = -89.0f;

    direction.x = glm::cos(glm::radians(scene.cameras[scene.cameraID]->yaw)) * glm::cos(glm::radians(scene.cameras[scene.cameraID]->pitch));
    direction.y = glm::sin(glm::radians(scene.cameras[scene.cameraID]->pitch));
    direction.z = glm::sin(glm::radians(scene.cameras[scene.cameraID]->yaw)) * glm::cos(glm::radians(scene.cameras[scene.cameraID]->pitch));

    scene.cameras[scene.cameraID]->target = glm::normalize(direction);

    // Élévation AE
    if(glfwGetKey(window,GLFW_KEY_E) == GLFW_PRESS) scene.cameras[scene.cameraID]->position += scene.cameras[scene.cameraID]->up * 20.0f * cameraSpeed;
    if(glfwGetKey(window,GLFW_KEY_Q) == GLFW_PRESS) scene.cameras[scene.cameraID]->position -= scene.cameras[scene.cameraID]->up * 20.0f * cameraSpeed;

    // Changement de caméra
    if(!isPressed && glfwGetKey(window,GLFW_KEY_C) == GLFW_PRESS) { 
        
        isPressed = true;
        scene.cameraID = scene.cameraID != scene.cameras.size()-1 ? scene.cameraID + 1 : 0;
    }
    if(glfwGetKey(window,GLFW_KEY_C) == GLFW_RELEASE) isPressed = false;
}