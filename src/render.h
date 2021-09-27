#pragma once

#include <algorithm>
#include <cstdlib>
#include <string>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

struct Vertex {
    Vertex();
    Vertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &uv);

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct Camera {
    glm::mat4 projMat;
    glm::mat4 viewMat;
    glm::vec3 cameraPos;
    glm::vec3 cameraDir;
    glm::vec3 cameraUp;
};

struct RenderObject {
    void initialize();
    void buildShader(const std::string &basename);
    void loadOBJ(const std::string &filename);
    void loadTexture(const std::string &filename);
    void draw(const Camera &camera, const glm::vec3 &lightPos, const glm::vec3 &lightLe);

    GLuint programId;
    GLuint vaoId;
    GLuint vboId;
    GLuint iboId;
    GLuint textureId;
    int bufferSize;

    glm::mat4 modelMat;
    glm::vec3 ambiColor;
    glm::vec3 diffColor;
    glm::vec3 specColor;
    float shininess;
};
