#pragma once

#include <array>
#include <vector>

#include "render.h"

static constexpr int NUM_CPS_IN_CURVE = 4;
static constexpr int COEFF_DIV = 1024;

enum LightType {
    ONE = 0,
    TWO = 1,
    THREE = 2,
    FOUR = 3,
    CAVITY = 4,
    CLIP = 5,
    QUAD = 6,
    CHAR = 7,
};

struct BezierLight : public RenderObject {
    void initialize();
    void createCPSmodel(LightType);
    void calcCPSworld();
    glm::vec3 bezierCurve(const int curve, const float t);

    void gaussianFilter(std::vector<std::vector<float>> &kernel, int kernelSize, float sigma);
    void createBezLightTex(const std::string &filename);

    void compBernCoeffs();
    void createBernCoeffTex();
    void drawBez(const Camera &camera);

    int numPoints;
    int numCurves;
    std::vector<glm::vec3> cpsModel;
    std::vector<glm::vec3> cpsWorld;
    std::vector<glm::vec3> samplePoints;
    std::array<glm::vec4, COEFF_DIV + 1> bernCoeffs;

    GLuint ptsVaoId;
    GLuint ptsVboId;

    glm::vec3 Le;
    glm::vec3 center;
    glm::vec2 size;
    glm::vec3 rotAngle;
    glm::vec3 translate;
    bool isTwoSided;
    bool isMove;
    bool isBezTexed;

    int texHeight;
    int texWidth;
    int marginSize;
    int maxLOD;

    GLuint bezLightTexId;
    GLuint bernCoeffTexId;


    glm::mat4 rotateX(float ax);
    glm::mat4 rotateY(float ay);
    glm::mat4 rotateZ(float az);
    glm::mat4 rotateXYZ(glm::vec3 rotAngle);
    glm::vec3 degToRad(glm::vec3 rotAngle);
};
