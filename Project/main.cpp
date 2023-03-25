#pragma comment(lib, "winmm.lib")
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <Windows.h>
#include <Mmsystem.h>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat3 lightDirMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightDirMatrixLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.3f;

GLboolean pressedKeys[1024];

// models
gps::Model3D ground;
gps::Model3D relief;
gps::Model3D water;
gps::Model3D background_trees;
gps::Model3D boat;
gps::Model3D sun;
gps::Model3D windmill_buildings;
gps::Model3D propeller1;
gps::Model3D propeller2;
gps::Model3D firs;
gps::Model3D forrest;
gps::Model3D glasses;
gps::Model3D village;
gps::Model3D water_drops[500];
GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader lightShader;

GLboolean isFirstMouse;
GLfloat xPrev = 1920 / 2.0;
GLfloat yPrev = 1080 / 2.0;

int retina_width, retina_height;

// Sky
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

// Fog functionality
int activateFog = 0;
GLfloat fogLoc;

// Mouse callback and movement
GLfloat pitch = 0.0f;
GLfloat yaw = 0.0f;

// Light
GLfloat lightAngle;
glm::mat4 lightRotation;
GLfloat ambientStrength = 0.2f;
GLfloat specularStrength = 0.5f;
GLfloat shininess = 30.0f;

// Switch Lighting
int spotLight = 0;
GLfloat spotLightLoc;
glm::vec3 spotLightPos;
GLuint spotLightPosLoc;
glm::vec3 spotLightDir;
GLuint spotLightDirLoc;
GLfloat spotLightCutOff = glm::cos(glm::radians(15.0f));
GLfloat spotLightOuterCutOff = glm::cos(glm::radians(20.0f));
GLuint spotLightCutOffLoc;
GLuint spotLightOuterCutOffLoc;

// Shadows
GLuint shadowMapFBO;
GLuint depthMapTexture;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

// Animations
GLfloat deltaAngle = 0.0f;
GLfloat xdir = 0.0f;
GLfloat ydir = 0.0f;
GLfloat steer;
GLfloat scaleFactor = 1.0f;
glm::mat4 modelPropeller;
glm::mat4 modelBoat;

// Rain
GLint nr_drops = 0;
GLboolean rainOn = false;
GLint dropx;
GLint dropy;
GLint gravity;
glm::mat4 modelRain;
int currentTime = 0;

// Presentation
GLfloat t = 0.0f;
GLboolean presentation = false;
GLint pass = 1;

GLfloat prevAmbient;
GLfloat prevSpecular;
GLfloat prevShininess;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
		std::string error;
		switch (errorCode)
        {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
    glfwGetFramebufferSize(window, &retina_width, &retina_height);
    
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Fog
    if (pressedKeys[GLFW_KEY_F])
    {
        activateFog = 1 - activateFog;
        glUniform1i(fogLoc, activateFog);
    }

    // Spotlight
    if (pressedKeys[GLFW_KEY_Y])
    {
        if (spotLight == 0)
        {
            spotLight = 1;
            prevAmbient = ambientStrength;
            prevSpecular = specularStrength;
            prevShininess = shininess;
            ambientStrength = 0.1f;
            specularStrength = 1.0f;
            shininess = 32.0f;
            // set the light direction (direction towards the light)
            lightDir = spotLightDir;
            lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
        }
        else
        {
            spotLight = 0;
            ambientStrength = prevAmbient;
            specularStrength = prevSpecular;
            shininess = prevShininess;
            //set the light direction (direction towards the light)
            lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
            lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
            // send light dir to shader
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
        }
        glUniform1i(spotLightLoc, spotLight);
        myBasicShader.useShaderProgram();

        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "specularStrength"), specularStrength);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "shininess"), shininess);
    }

	if (key >= 0 && key < 1080)
    {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    //TODO
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
    {
        isFirstMouse = true;
        return;
    }
    if (isFirstMouse == true)
    {
        xPrev = xpos;
        yPrev = ypos;
        isFirstMouse = false;
    }
    float xoffset = xpos - xPrev;
    float yoffset = yPrev - ypos;
    xPrev = xpos;
    yPrev = ypos;

    xoffset *= cameraSpeed;
    yoffset *= cameraSpeed;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

glm::vec3 calcPoint(glm::vec3 Point0, glm::vec3 Point1, float t)
{
    glm::vec3 point;
    point.x = (1 - t) * Point0.x + t * Point1.x;
    point.y = (1 - t) * Point0.y + t * Point1.y;
    point.z = (1 - t) * Point0.z + t * Point1.z;

    return point;
}

void moveCamera(glm::vec3 Position0, glm::vec3 Position1, glm::vec3 Target0, glm::vec3 Target1)
{
    if (t <= 1.0f)
    {
        glm::vec3 position, direction;
        position = calcPoint(Position0, Position1, t);
        direction = calcPoint(Target0, Target1, t);
        myCamera.moveAnimation(position, direction);
        t += 0.01f;
    }
    else
    {
        t = 0.0f;
        pass++;
        if (pass == 24) {
            pass = 1;
        }
    }
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_W])
    {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S])
    {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A])
    {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D])
    {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q])
    {
        angle -= 0.5f;
        if (angle < 0.0f) {
            angle += 360.0f;
        }
    }

    if (pressedKeys[GLFW_KEY_E])
    {
        angle += 0.5f;
        if (angle > 360.0f) {
            angle -= 360.0f;
        }
    }

    // Light
    if (pressedKeys[GLFW_KEY_K])
    {
        lightAngle -= 0.3f;
        if (lightAngle < 0.0f) {
            lightAngle += 360.0f;
        }
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }
    if (pressedKeys[GLFW_KEY_L])
    {
        lightAngle += 0.3f;
        if (lightAngle > 360.0f) {
            lightAngle -= 360.0f;
        }
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }

    // Solid View
    if (pressedKeys[GLFW_KEY_Z]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // Wireframe View
    if (pressedKeys[GLFW_KEY_X]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    // Poligonal View
    if (pressedKeys[GLFW_KEY_C]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    // Light Intensity
    if (pressedKeys[GLFW_KEY_N])
    {
        myBasicShader.useShaderProgram();
        ambientStrength -= 0.02f;
        specularStrength -= 0.02f;
        shininess -= 1.0f;

        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "specularStrength"), specularStrength);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "shininess"), shininess);
    }
    if (pressedKeys[GLFW_KEY_M])
    {
        myBasicShader.useShaderProgram();
        ambientStrength += 0.02f;
        specularStrength += 0.02f;
        shininess += 1.0f;

        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "specularStrength"), specularStrength);
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "shininess"), shininess);
    }

    // Boat Movement
    if (pressedKeys[GLFW_KEY_UP]) {
        xdir -= 0.3f;
    }
    if (pressedKeys[GLFW_KEY_DOWN]) {
        xdir += 0.3f;
    }
    if (pressedKeys[GLFW_KEY_LEFT]) {
        ydir += 0.3f;
    }
    if (pressedKeys[GLFW_KEY_RIGHT]) {
        ydir -= 0.3f;
    }
    if (pressedKeys[GLFW_KEY_R])
    {
        steer += 1.0f;
        if (steer == 360.0f) {
            steer -= 360.0f;
        }
    }
    if (pressedKeys[GLFW_KEY_T])
    {
        steer -= 1.0f;
        if (steer == 0.0f) {
            steer += 360.0f;
        }
    }
    if (pressedKeys[GLFW_KEY_LEFT_BRACKET])
    {
        scaleFactor -= 0.05f;
        if (scaleFactor < 0.0f) {
            scaleFactor = 0.0f;
        }
    }
    if (pressedKeys[GLFW_KEY_RIGHT_BRACKET]) {
        scaleFactor += 0.05f;
    }

    // Rain
    if (pressedKeys[GLFW_KEY_O])
    {
        nr_drops = 0;
        rainOn = false;
        // set the light direction (direction towards the light)
        lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
        // send light dir to shader
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    }
    if (pressedKeys[GLFW_KEY_P])
    {
        nr_drops = 500;
        rainOn = true;
        // set the light direction (direction towards the light)
        lightDir = glm::vec3(20.0f, 1.0f, 10.0f);
        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
        // send light dir to shader
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
    }

    if (pressedKeys[GLFW_KEY_ENTER]) {
        presentation = true;
    }
    else {
        presentation = false;
    }
    if (presentation == true)
    {
        glm::vec3 P1(11.8888, -0.0482321, 31.83), T1(11.9096, 0.092669, 30.8402);
        glm::vec3 P2(19.7411, 0.895827, -8.26359), T2(19.8037, 0.979505, -9.25811);
        glm::vec3 P3(11.9111, 1.86421, -41.001), T3(11.8902, 1.927, -41.9989);
        glm::vec3 P4(13.609, 2.23287, -46.1319), T4(14.4667, 2.31654, -46.6391);
        glm::vec3 P5(-11.1195, 0.679578, -48.291), T5(-12.053, 0.690049, -47.9326);
        glm::vec3 P6(-39.3828, -0.526004, -22.103), T6(-40.3461, -0.421475, -21.8557);
        glm::vec3 P7(-43.3024, -0.201149, -20.8874), T7(-43.8325, -0.00862692, -21.7132);
        glm::vec3 P8(-63.4617, -0.930678, -19.221), T8(-64.3712, -0.836569, -19.6259);
        glm::vec3 P9(-64.3952, -0.874214, -18.4637), T9(-64.2393, -0.790536, -17.4794);
        glm::vec3 P10(-74.8726, -0.865777, -22.0949), T10(-75.8514, -0.761248, -21.9188);
        glm::vec3 P11(-79.5712, -0.654077, -20.0032), T11(-79.6649, -0.565183, -19.0116);
        glm::vec3 P12(-81.0345, -1.04664, 3.86129), T12(-81.0761, -0.931703, 4.85379);
        glm::vec3 P13(-78.8212, -0.473802, 9.34486), T13(-77.845, -0.332901, 9.50997);
        glm::vec3 P14(-26.0426, 0.890427, 17.4085), T14(-25.0662, 0.979322, 17.6054);
        glm::vec3 P15(-2.77109, -0.204, 25.0123), T15(-1.78849, -0.021765, 24.9763);
        glm::vec3 P16(50.1607, 1.43331, 29.9035), T16(51.1121, 1.57939, 30.1746);
        glm::vec3 P17(54.7962, 2.2903, 27.2212), T17(55.1871, 2.52375, 26.3308);
        glm::vec3 P18(84.4877, 2.74658, 49.8383), T18(85.1971, 2.80937, 50.5403);
        glm::vec3 P19(92.133, 2.11259, 67.5124), T19(91.6989, 2.27419, 68.3986);
        glm::vec3 P20(74.8913, 5.51412, 97.8472), T20(74.2706, 5.63426, 98.622);
        glm::vec3 P21(69.6705, 6.425, 107.589), T21(68.6966, 6.55552, 107.775);
        
        switch (pass)
        {
        case 1:
            moveCamera(P1, P2, T1, T2);
            break;
        case 2:
            moveCamera(P2, P3, T2, T3);
            break;
        case 3:
            moveCamera(P3, P4, T3, T4);
            break;
        case 4:
            moveCamera(P4, P3, T4, T3);
            break;
        case 5:
            moveCamera(P3, P5, T3, T5);
            break;
        case 6:
            moveCamera(P5, P6, T5, T6);
            break;
        case 7:
            moveCamera(P6, P7, T6, T7);
            break;
        case 8:
            moveCamera(P7, P8, T7, T8);
            break;
        case 9:
            moveCamera(P8, P9, T8, T9);
            break;
        case 10:
            moveCamera(P9, P8, T9, T8);
            break;
        case 11:
            moveCamera(P8, P10, T8, T10);
            break;
        case 12:
            moveCamera(P10, P11, T10, T11);
            break;
        case 13:
            moveCamera(P11, P12, T11, T12);
            break;
        case 14:
            moveCamera(P12, P13, T12, T13);
            break;
        case 15:
            moveCamera(P13, P14, T13, T14);
            break;
        case 16:
            moveCamera(P14, P15, T14, T15);
            break;
        case 17:
            moveCamera(P15, P16, T15, T16);
            break;
        case 18:
            moveCamera(P16, P17, T16, T17);
            break;
        case 19:
            moveCamera(P17, P16, T17, T16);
            break;
        case 20:
            moveCamera(P16, P18, T16, T18);
            break;
        case 21:
            moveCamera(P18, P19, T18, T19);
            break;
        case 22:
            moveCamera(P19, P20, T19, T20);
            break;
        case 23:
            moveCamera(P20, P21, T20, T21);
        }
    }

    // End Application
    if (pressedKeys[GLFW_KEY_ESCAPE]) {
        exit(0);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void setWindowCallbacks()
{
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState()
{
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_COLOR_EXT); // for transparency
}

void initModels()
{
    ground.LoadModel("models/ground/ground.obj", "models/ground\\");
    relief.LoadModel("models/relief/relief.obj", "models/relief\\");
    water.LoadModel("models/water/water.obj", "models/water\\");
    background_trees.LoadModel("models/background_trees/background_trees.obj", "models/background_trees\\");
    boat.LoadModel("models/boat/boat.obj", "models/boat\\");
    sun.LoadModel("models/sun/sun.obj", "models/sun\\");
    windmill_buildings.LoadModel("models/windmill_buildings/windmill_buildings.obj", "models/windmill_buildings\\");
    propeller1.LoadModel("models/propeller1/propeller1.obj", "models/propeller1\\");
    propeller2.LoadModel("models/propeller2/propeller2.obj", "models/propeller2\\");
    glasses.LoadModel("models/glasses/glasses.obj", "models/glasses\\");
    village.LoadModel("models/village/village.obj", "models/village\\");
    firs.LoadModel("models/firs/firs.obj", "models/firs\\");
    forrest.LoadModel("models/forrest/forrest.obj", "models/forrest\\");
    for (int i = 0; i < 500; i++) {
        water_drops[i].LoadModel("models/water_drop/water_drop.obj", "models/water_drop\\");
    }
}

void initShaders()
{
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    lightShader.loadShader("shaders/lightShader.vert", "shaders/lightShader.frag");
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
}

void initUniforms()
{
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	// set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightDirMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDirMatrix");

    fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "activateFog");
    glUniform1i(fogLoc, activateFog);

    spotLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "position");
    glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(myCamera.getCameraPosition()));
    spotLightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "direction");
    glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(myCamera.getCameraFrontDirection()));
    spotLightCutOffLoc = glGetUniformLocation(myBasicShader.shaderProgram, "cutOff");
    glUniform1f(spotLightCutOffLoc, spotLightCutOff);
    spotLightOuterCutOffLoc = glGetUniformLocation(myBasicShader.shaderProgram, "outerCutOff");
    glUniform1f(spotLightOuterCutOffLoc, spotLightOuterCutOff);

    spotLightLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLight");
    glUniform1i(spotLightLoc, spotLight);

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    skyboxShader.useShaderProgram();
}

void initFBO()
{
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    // generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    // create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
    glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
    glm::mat4 lightView = glm::lookAt(200.0f * lightDirTr, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const GLfloat near_plane = -50.0f, far_plane = 600.0f;
    glm::mat4 lightProjection = glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, near_plane, far_plane);

    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void renderObjects(gps::Shader shader, bool depthPass)
{
    // select active shader program
    shader.useShaderProgram();
    
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass)
    {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw models
    background_trees.Draw(shader);
    windmill_buildings.Draw(shader);
    firs.Draw(shader);
    forrest.Draw(shader);
    village.Draw(shader);

    // Rotate Boat
    modelBoat = glm::translate(model, glm::vec3(-33.963f + xdir, -9.4218f, 193.7f + ydir));
    modelBoat = glm::rotate(modelBoat, glm::radians(steer), glm::vec3(0.0f, 1.0f, 0.0f));
    modelBoat = glm::translate(modelBoat, glm::vec3(-(-33.963f + xdir), 9.4218f, -(193.7f + ydir)));
    // Scale Boat
    modelBoat = glm::translate(modelBoat, glm::vec3(-33.963f + xdir, -9.4218f, 193.7f + ydir));
    modelBoat = glm::scale(modelBoat, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
    modelBoat = glm::translate(modelBoat, glm::vec3(-(-33.963f + xdir), 9.4218f, -(193.7f + ydir)));
    // Translate Boat
    modelBoat = glm::translate(modelBoat, glm::vec3(xdir, 0.0f, ydir));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBoat));
    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    boat.Draw(shader);

    deltaAngle += 1.0f;
    if (deltaAngle == 360.0f) {
        deltaAngle = 0.0f;
    }

    modelPropeller = glm::rotate(model, glm::radians(deltaAngle), glm::vec3(147.76f, 16.104f, 13.07f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPropeller));
    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    propeller1.Draw(shader);

    modelPropeller = glm::rotate(model, glm::radians(deltaAngle), glm::vec3(-5.7224, 7.6023f, 254.15f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPropeller));
    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    propeller2.Draw(shader);

    for (int i = 0; i < nr_drops; i++)
    {
        dropx = rand() % (286 - (-308)) + (-308);
        dropy = rand() % (409 - (-296)) + (-296);
        gravity = rand() % 50;
        modelRain = glm::translate(model, glm::vec3(dropx, 0.0f, dropy));
        modelRain = glm::translate(modelRain, glm::vec3(1.0f, gravity, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelRain));
        // do not send the normal matrix if we are rendering in the depth map
        if (!depthPass) {
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }
        water_drops[i].Draw(shader);
    }
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render the scene
    // render the scene to the depth buffer
    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderObjects(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
    glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));
    myBasicShader.useShaderProgram();

    // bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    
    // render objects
    mySkyBox.Draw(skyboxShader, view, projection);

    // draw the light onto the sun
    lightShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = lightRotation;
    model = glm::translate(model, 1.0f * lightDir);
    model = glm::scale(model, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    sun.Draw(lightShader);
    
    renderObjects(myBasicShader, false);

    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    relief.Draw(myBasicShader);
    ground.Draw(myBasicShader);

    glEnable(GL_BLEND);
    water.Draw(myBasicShader);
    glasses.Draw(myBasicShader);
    glDisable(GL_BLEND);

    if (rainOn == true)
    {
        if (currentTime % 500 == 50 || currentTime % 500 == 260 || currentTime % 500 == 290 || currentTime % 500 == 410)
        {
            PlaySound(TEXT("thunder.wav"), NULL, SND_FILENAME | SND_ASYNC);
            prevAmbient = ambientStrength;
            prevSpecular = specularStrength;
            prevShininess = shininess;
            ambientStrength = 1.0f;
            specularStrength = 1.0f;
            shininess = 0.0f;
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "ambientStrength"), ambientStrength);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "specularStrength"), specularStrength);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "shininess"), shininess);
        }
        if (currentTime % 500 == 60 || currentTime % 500 == 270 || currentTime % 500 == 300 || currentTime % 500 == 420)
        {
            ambientStrength = prevAmbient;
            specularStrength = prevSpecular;
            shininess = prevShininess;
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "ambientStrength"), ambientStrength);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "specularStrength"), specularStrength);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "shininess"), shininess);
        }
    }
}

void cleanup()
{
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[])
{
    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.png");
    faces.push_back("skybox/left.png");
    faces.push_back("skybox/top.png");
    faces.push_back("skybox/bottom.png");
    faces.push_back("skybox/back.png");
    faces.push_back("skybox/front.png");
    mySkyBox.Load(faces);

    initOpenGLState();
	initModels();
	initShaders();
    initUniforms();
    initFBO();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 20.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow()))
    {
        Sleep(1);
        currentTime++;
        if (currentTime == 500) {
            currentTime = 0;
        }

        processMovement();
        glCheckError();
	    renderScene();
        glCheckError();

		glfwPollEvents();
        glCheckError();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
