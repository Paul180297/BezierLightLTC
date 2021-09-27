#include <algorithm>
#include <iostream>

#include <glad/gl.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include "bezierLight.h"
#include "common.h"
#include "openmp.h"

static constexpr int MARGIN_SIZE = 0;
static constexpr int MAXDIST = 55;
static constexpr int OVERLAP = 7;

namespace {

int C(int n, int m) {
    m = std::min(m, n - m);
    if (m == 0) {
        return 1;
    }
    if (m == 1) {
        return n;
    }

    int ret = 1;
    for (int i = 0; i < m; i++) {
        ret *= (n - i) / (i + 1);
    }

    return ret;
}

float bernstein(int n, int i, double t) {
    return C(n, i) * std::pow(t, i) * std::pow(1.0 - t, n - i);
}

}  // anonymous namespace

void BezierLight::initialize() {
    numPoints = 0;  // initialize with minumum value
    numCurves = 0;
    Le = glm::vec3(1.0f);
    center = glm::vec3(0.0f);
    size = glm::vec2(1.0f);
    rotAngle = glm::vec3(0.0f);
    translate = glm::vec3(0.0f);

    texHeight = 0;
    texWidth = 0;
    marginSize = 0;
    maxLOD = 0;

    isTwoSided = false;
    isBezTexed = false;

    bezLightTexId = -1;
    bernCoeffTexId = -1;
}

void BezierLight::createCPSmodel(LightType type) {
    // create cps in normalize model space [-1, 1]
    cpsModel.clear();
    cpsModel.shrink_to_fit();

    switch (type) {
    case 0: {
        cpsModel.push_back(glm::vec3(0.0f, 0.65f, 0.0f));
        cpsModel.push_back(glm::vec3(1.6f, -0.95f, 0.0f));
        cpsModel.push_back(glm::vec3(-1.6f, -0.95f, 0.0f));
        cpsModel.push_back(cpsModel[0]);
    } break;

    case 1: {
        const float scale = 0.9f;
        cpsModel.push_back(glm::vec3(-0.8f, 0.6f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(-0.4375f, 1.4f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(0.4375f, -0.0f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(0.8f, 0.6f, 0.0f) * scale);
        cpsModel.push_back(cpsModel[3]);
        cpsModel.push_back(glm::vec3(0.125f, -1.85f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(-0.125f, 0.30f, 0.0f) * scale);
        cpsModel.push_back(cpsModel[0]);
    } break;

    case 2: {
        const float scale = 0.85f;
        cpsModel.push_back(glm::vec3(-0.1f, 0.10f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(-0.8f, 1.1f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(0.8f, 1.1f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(0.1f, 0.1f, 0.0f) * scale);
        cpsModel.push_back(cpsModel[3]);
        cpsModel.push_back(glm::vec3(0.8f, 0.1f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(0.8f, -0.90f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(0.0f, -0.40f, 0.0f) * scale);
        cpsModel.push_back(cpsModel[7]);
        cpsModel.push_back(glm::vec3(-0.8f, -0.90f, 0.0f) * scale);
        cpsModel.push_back(glm::vec3(-0.8f, 0.10f, 0.0f) * scale);
        cpsModel.push_back(cpsModel[0]);
    } break;

    case 3: {
        cpsModel.push_back(glm::vec3(-0.5f, 0.5f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.16666f, 1.0f, 0.0f));
        cpsModel.push_back(glm::vec3(0.16666f, 0.2f, 0.0f));
        cpsModel.push_back(glm::vec3(0.5f, 0.5f, 0.0f));
        cpsModel.push_back(cpsModel[3]);
        cpsModel.push_back(glm::vec3(1.0f, 0.19f, 0.0f));
        cpsModel.push_back(glm::vec3(0.4f, -0.3f, 0.0f));
        cpsModel.push_back(glm::vec3(0.5f, -0.5f, 0.0f));
        cpsModel.push_back(cpsModel[7]);
        cpsModel.push_back(glm::vec3(0.0f, 0.2f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.3f, -0.4f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.5f, -0.5f, 0.0f));
        cpsModel.push_back(cpsModel[11]);
        cpsModel.push_back(glm::vec3(-1.2f, -0.2f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.1f, 0.15f, 0.0f));
        cpsModel.push_back(cpsModel[0]);
    } break;

    case 4: {
        cpsModel.push_back(glm::vec3(0.0f, 0.65f, 0.0f));
        cpsModel.push_back(glm::vec3(0.7f, 0.3f, 0.0f));
        cpsModel.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        cpsModel.push_back(glm::vec3(0.2f, -0.55f, 0.0f));
        cpsModel.push_back(cpsModel[3]);
        cpsModel.push_back(glm::vec3(0.4f, -0.4f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.0f, -0.3f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, -0.1f, 0.0f));
        cpsModel.push_back(cpsModel[7]);
        cpsModel.push_back(glm::vec3(0.0f, -0.3f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.4f, -0.4f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.2f, -0.55f, 0.0f));
        cpsModel.push_back(cpsModel[11]);
        cpsModel.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.7f, 0.3f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, 0.65f, 0.0f));
        cpsModel.push_back(cpsModel[15]);
        cpsModel.push_back(glm::vec3(0.0f, 0.5f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, 0.45f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, 0.4f, 0.0f));
        cpsModel.push_back(cpsModel[19]);
        cpsModel.push_back(glm::vec3(-0.5f, 0.0f, 0.0f));
        cpsModel.push_back(glm::vec3(0.5f, 0.0f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, 0.4f, 0.0f));
        cpsModel.push_back(cpsModel[23]);
        cpsModel.push_back(glm::vec3(0.0f, 0.45f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, 0.5f, 0.0f));
        cpsModel.push_back(cpsModel[0]);
    } break;

    case 5: {
        cpsModel.push_back(glm::vec3(-0.8f, -0.64f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.4f, 0.8f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.2f, -0.4f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, -0.4f, 0.0f));
        cpsModel.push_back(cpsModel[3]);
        cpsModel.push_back(glm::vec3(0.2f, -0.4f, 0.0f));
        cpsModel.push_back(glm::vec3(0.4f, 0.8f, 0.0f));
        cpsModel.push_back(glm::vec3(0.8f, -0.64f, 0.0f));
        cpsModel.push_back(cpsModel[7]);
        cpsModel.push_back(glm::vec3(0.0f, -0.64f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, -0.64f, 0.0f));
        cpsModel.push_back(cpsModel[0]);
    } break;

    case 6: {
        cpsModel.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        cpsModel.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
        cpsModel.push_back(cpsModel[3]);
        cpsModel.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        cpsModel.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        cpsModel.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
        cpsModel.push_back(cpsModel[7]);
        cpsModel.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
        cpsModel.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
        cpsModel.push_back(cpsModel[11]);
        cpsModel.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
        cpsModel.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
        cpsModel.push_back(cpsModel[0]);
    } break;

    case 7: {
        // Line #0:
        cpsModel.push_back(glm::vec3(-0.3855f, -0.5181f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.1042f, -0.0200f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.1042f, -0.0200f, 0.0f));
        cpsModel.push_back(glm::vec3(0.1770f, 0.4780f, 0.0f));
        // Line #1:
        cpsModel.push_back(glm::vec3(0.1770f, 0.4780f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0142f, 0.4780f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0142f, 0.4780f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.1487f, 0.4780f, 0.0f));
        // CubicBezier #2:
        cpsModel.push_back(glm::vec3(-0.1487f, 0.4780f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.1834f, 0.4820f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.2177f, 0.4683f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.2400f, 0.4414f, 0.0f));
        // CubicBezier #3:
        cpsModel.push_back(glm::vec3(-0.2400f, 0.4414f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.2571f, 0.4165f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.3572f, 0.2905f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.3572f, 0.2905f, 0.0f));
        // Line #4:
        cpsModel.push_back(glm::vec3(-0.3572f, 0.2905f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.3535f, 0.4216f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.3535f, 0.4216f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.3499f, 0.5527f, 0.0f));
        // Line #5:
        cpsModel.push_back(glm::vec3(-0.3499f, 0.5527f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0056f, 0.5527f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0056f, 0.5527f, 0.0f));
        cpsModel.push_back(glm::vec3(0.3611f, 0.5527f, 0.0f));
        // Line #6:
        cpsModel.push_back(glm::vec3(0.3611f, 0.5527f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0791f, 0.0549f, 0.0f));
        cpsModel.push_back(glm::vec3(0.0791f, 0.0549f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.2029f, -0.4429f, 0.0f));
        // Line #7:
        cpsModel.push_back(glm::vec3(-0.2029f, -0.4429f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.0137f, -0.4429f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.0137f, -0.4429f, 0.0f));
        cpsModel.push_back(glm::vec3(0.1755f, -0.4429f, 0.0f));
        // CubicBezier #8:
        cpsModel.push_back(glm::vec3(0.1755f, -0.4429f, 0.0f));
        cpsModel.push_back(glm::vec3(0.2106f, -0.4470f, 0.0f));
        cpsModel.push_back(glm::vec3(0.2454f, -0.4333f, 0.0f));
        cpsModel.push_back(glm::vec3(0.2683f, -0.4063f, 0.0f));
        // CubicBezier #9:
        cpsModel.push_back(glm::vec3(0.2683f, -0.4063f, 0.0f));
        cpsModel.push_back(glm::vec3(0.2854f, -0.3819f, 0.0f));
        cpsModel.push_back(glm::vec3(0.3855f, -0.2554f, 0.0f));
        cpsModel.push_back(glm::vec3(0.3855f, -0.2554f, 0.0f));
        // Line #10:
        cpsModel.push_back(glm::vec3(0.3855f, -0.2554f, 0.0f));
        cpsModel.push_back(glm::vec3(0.3816f, -0.3867f, 0.0f));
        cpsModel.push_back(glm::vec3(0.3816f, -0.3867f, 0.0f));
        cpsModel.push_back(glm::vec3(0.3777f, -0.5181f, 0.0f));
        // Line #11:
        cpsModel.push_back(glm::vec3(0.3777f, -0.5181f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.0039f, -0.5181f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.0039f, -0.5181f, 0.0f));
        cpsModel.push_back(glm::vec3(-0.3855f, -0.5181f, 0.0f));
    } break;
    }

    this->numPoints = (int) cpsModel.size();
    this->numCurves = numPoints / NUM_CPS_IN_CURVE;
    for (auto v : cpsModel) {
        center += v / (float) numPoints;
    }

    // compute sample points on boundary curve
    samplePoints.clear();
    samplePoints.push_back(glm::vec3(0.0f));
    const int nSplit = 32;
    for (int i = 0; i < this->numCurves; i++) {
        for (int j = 0; j < nSplit; j++) {
            glm::vec3 p(0.0f);
            const float t = (float) j / (float) nSplit;
            for (int d = 0; d < NUM_CPS_IN_CURVE; d++) {
                p += bernstein(NUM_CPS_IN_CURVE - 1, d, t) * cpsModel[i * NUM_CPS_IN_CURVE + d];
            }
            samplePoints.push_back(p);
        }
    }
    samplePoints.push_back(samplePoints[1]);

    // create VAO for sample points
    glGenVertexArrays(1, &ptsVaoId);
    glBindVertexArray(ptsVaoId);

    glGenBuffers(1, &ptsVboId);
    glBindBuffer(GL_ARRAY_BUFFER, ptsVboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * samplePoints.size(), samplePoints.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    glBindVertexArray(0);

    // default translation parameters
    size = glm::vec2(2.0f);
    rotAngle = 90.0f * glm::vec3(0.0f, 0.0f, 0.0f);
    translate = glm::vec3(0.0f, 1.30f, 0.0f);
}

void BezierLight::calcCPSworld() {
    // update transformation
    modelMat = glm::translate(translate) *
               rotateXYZ(degToRad(rotAngle)) *
               glm::scale(glm::vec3(size, 1.0f));

    // compute control points in world space
    cpsWorld.resize(numPoints);
    for (int i = 0; i < numPoints; i++) {
        glm::vec4 p = modelMat * glm::vec4(cpsModel[i], 1.0f);
        // Avoid numerical unstability in algebraic clipping
        p.y = p.y > 0.0 ? p.y + 1.0e-3 : p.y - 1.0e-3;
        cpsWorld[i] = glm::vec3(p.x, p.y, p.z);
    }

    // compute barycenter of area light
    glm::vec4 p = modelMat * glm::vec4(center, 1.0f);
    center = glm::vec3(p.x, p.y, p.z);
}

glm::mat4 BezierLight::rotateX(float ax) {
    glm::mat4 xRotMat = glm::rotate(ax, glm::vec3(1.0f, 0.0f, 0.0f));
    return xRotMat;
}

glm::mat4 BezierLight::rotateY(float ay) {
    glm::mat4 yRotMat = glm::rotate(ay, glm::vec3(0.0f, 1.0f, 0.0f));
    return yRotMat;
}

glm::mat4 BezierLight::rotateZ(float az) {
    glm::mat4 zRotMat = glm::rotate(az, glm::vec3(0.0f, 0.0f, 1.0f));
    return zRotMat;
}

glm::mat4 BezierLight::rotateXYZ(glm::vec3 rotAngle) {
    glm::mat4 rotMat = rotateZ(rotAngle.z) * rotateY(rotAngle.y) * rotateX(rotAngle.x);
    return rotMat;
}

glm::vec3 BezierLight::degToRad(glm::vec3 rotAngle) {
    return (float) (Pi / 180.0) * rotAngle;
}

glm::vec3 BezierLight::bezierCurve(const int curve, const float t) {
    glm::vec3 vs[NUM_CPS_IN_CURVE];
    for (int i = 0; i < NUM_CPS_IN_CURVE; i++) {
        vs[i] = cpsModel[curve * NUM_CPS_IN_CURVE + i];
    }

    for (int i = NUM_CPS_IN_CURVE - 1; i >= 1; i--) {
        for (int j = 0; j < i; j++) {
            vs[j] = glm::mix(vs[j], vs[j + 1], t);
        }
    }

    return vs[0];
}

void BezierLight::gaussianFilter(std::vector<std::vector<float>> &kernel, int kernelSize, float sigma) {
    const float twicedSigmaSquared = 2.0f * sigma * sigma;

    kernel = std::vector<std::vector<float>>(kernelSize, std::vector<float>(kernelSize, 0.0f));  // deep copy

    if (kernelSize % 2 == 1) {
        // generating kernel for odd kernelSize
        // left up to right down
        const int offset = kernelSize / 2;
        float sum = 0.0f;
        for (int kernelY = -offset; kernelY <= offset; kernelY++) {  // 10 11 12 13 14
            for (int kernelX = -offset; kernelX <= offset; kernelX++) {
                const float rSquared = kernelX * kernelX + kernelY * kernelY;
                const float weight = exp(-rSquared / twicedSigmaSquared);
                kernel[kernelY + offset][kernelX + offset] = weight;
                sum += weight;
            }
        }

        // normalizing
        for (int kernelY = 0; kernelY < kernelSize; kernelY++) {
            for (int kernelX = 0; kernelX < kernelSize; kernelX++) {
                kernel[kernelY][kernelX] /= sum;
            }
        }
    } else {
        Error("kernelSize is even number, invalid size!!");
    }
}

void BezierLight::createBezLightTex(const std::string &filename) {
    // load image file
    int channels;
    unsigned char *bytes = stbi_load(filename.c_str(), &this->texWidth, &this->texHeight, &channels, STBI_rgb_alpha);

    if (!bytes) {
        fprintf(stderr, "Failed to load image file: %s\n", filename.c_str());
        exit(1);
    }
    this->marginSize = MARGIN_SIZE;
    this->maxLOD = int(std::log2(texWidth + 2 * marginSize));

    // prepare texture storage
    GLenum target = GL_TEXTURE_2D;
    GLenum filter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum address = GL_CLAMP_TO_EDGE;

    glGenTextures(1, &bezLightTexId);
    glBindTexture(target, bezLightTexId);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, address);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, address);
    glTexImage2D(target, 0, GL_RGBA8, this->texWidth, this->texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glGenerateMipmap(target);

    // generate prefiltered LOD texture
    if (texWidth != texHeight || pow(2, std::log2(texHeight)) != texHeight) {
        Error("invalid texture size, cannot compute maxLOD");
    }

    float *Pbytes = new float[4 * texWidth * texHeight];
    for (int i = 0; i < 4 * texWidth * texHeight; i++) {
        Pbytes[i] = float(bytes[i]) / 255.0f;
    }

    // clip texture by bezier-curve shape and generate distance map
    for (int row = 0; row < texHeight; row++) {
        for (int col = 0; col < texWidth; col++) {
            const int texelIndex = 4 * (row * texWidth + col);
            glm::vec3 texelUV = glm::vec3(float(col) / (texWidth - 1), 1.0f - float(row) / (texHeight - 1), 0.0);

            const int NDIV = 32;
            const float dt = 1.0f / NDIV;
            glm::vec3 dir = glm::vec3(0.0f);
            float sumAngle = 0.0f;
            for (int curve = 0; curve < numCurves; curve++) {
                // change [-1, 1] space to [0, 1] space
                glm::vec3 v0 = 0.5f * bezierCurve(curve, 0.0) + glm::vec3(0.5f, 0.5f, 0.0);
                glm::vec3 e0 = v0 - texelUV;
                for (int div = 0; div < NDIV; div++) {
                    const float t1 = (div + 1) * dt;

                    const glm::vec3 v1 = 0.5f * bezierCurve(curve, t1) + glm::vec3(0.5f, 0.5f, 0.0);
                    const glm::vec3 e1 = v1 - texelUV;

                    // check cross direction
                    float tmp;
                    glm::vec3 crs = glm::cross(e0, e1);
                    if (curve == 0 && div == 0) {
                        tmp = 1.0f;
                        dir = crs;
                    } else {
                        float inner = dot(crs, dir);
                        tmp = glm::sign(inner);
                    }

                    float l0l1 = length(e0) * length(e1) + 0.00001;
                    float angle = acos(dot(e0, e1) / l0l1);
                    sumAngle += tmp * angle;

                    v0 = v1;
                    e0 = e1;
                }
            }

            if (glm::abs(sumAngle) < Pi) {
                Pbytes[texelIndex + 0] = 0.0f;
                Pbytes[texelIndex + 1] = 0.0f;
                Pbytes[texelIndex + 2] = 0.0f;
                Pbytes[texelIndex + 3] = 0.0f;  // sign of outside of texture
            }
        }
    }

    // Save
    for (int i = 0; i < texWidth * texHeight; i++) {
        if (Pbytes[4 * i + 3] == 0.0f) {
            bytes[4 * i + 0] = 0;
            bytes[4 * i + 1] = 0;
            bytes[4 * i + 2] = 0;
            bytes[4 * i + 3] = 0;
        }
    }

    // id needed, export png for checking clipped texture
    // stbi_write_png(std::string("clipped_texture.png").c_str(), texWidth, texHeight, 4, bytes, 0);
    stbi_image_free(bytes);

    // create Gaussian filter for inside of Bezier curve
    std::vector<std::vector<float>> kernel;
    unsigned int kernelSize = 15;
    const float sigma = 9.0f;
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }
    gaussianFilter(kernel, kernelSize, sigma);

    // create Gaussian filters for outside of Bezier curve
    std::vector<std::vector<std::vector<float>>> outKernels;
    std::vector<unsigned int> outKernelSizes;
    for (int i = OVERLAP; i <= MAXDIST; i++) {
        std::vector<std::vector<float>> outKernel;
        const float outSigma = i;
        const uint32_t outKernelSize = 2 * i + 1;

        gaussianFilter(outKernel, outKernelSize, outSigma);

        outKernels.emplace_back(outKernel);
        outKernelSizes.emplace_back(outKernelSize);
    }

    int LODfactor = 1;
    for (int LOD = 0; LOD <= maxLOD; LOD++) {
        const int LODwidth = texWidth / LODfactor;
        const int LODheight = texHeight / LODfactor;
        float *Sbytes = new float[4 * LODwidth * LODheight];
        if (LOD == 0) {
            memcpy(Sbytes, Pbytes, sizeof(float) * 4 * texWidth * texHeight);
        } else if (LOD > 0) {
            // texture size change
            unsigned int SpixelPos = 0;
            const int Pwidth = LODwidth * 2;
            const int Pheight = LODheight * 2;
            for (int row = 0; row < Pheight; row += 2) {
                for (int col = 0; col < Pwidth; col += 2) {
                    // calculate pixelPos
                    int PpixelPos = row * Pwidth + col;

                    // take average for RGBA
                    for (int RGB = 0; RGB < 3; RGB++) {
                        int dataPos[4];
                        dataPos[0] = 4 * PpixelPos + RGB;                 // left up
                        dataPos[1] = 4 * (PpixelPos + 1) + RGB;           // right up
                        dataPos[2] = 4 * (PpixelPos + Pwidth) + RGB;      // left down
                        dataPos[3] = 4 * (PpixelPos + Pwidth + 1) + RGB;  // right down

                        float ave = 0.25f * (Pbytes[dataPos[0]] + Pbytes[dataPos[1]] + Pbytes[dataPos[2]] + Pbytes[dataPos[3]]);
                        Sbytes[4 * SpixelPos + RGB] = ave;
                    }
                    Sbytes[4 * SpixelPos + 3] = 1.0f;

                    SpixelPos++;
                }
            }
        }

        // for next loop
        memcpy(Pbytes, Sbytes, sizeof(float) * 4 * LODwidth * LODheight);

        float *Gbytes = new float[4 * LODwidth * LODheight];
        omp_parallel_for(int row = 0; row < LODheight; row++) {
            for (int col = 0; col < LODwidth; col++) {
                // calculate SpixelPos from row and col
                unsigned int SpixelPos = row * LODwidth + col;

                if (LOD == 0) {
                    // inside of Bezier curve
                    if (Sbytes[4 * SpixelPos + 3] != 0.0f) {  // just copy
                        Gbytes[4 * SpixelPos + 0] = Sbytes[4 * SpixelPos + 0];
                        Gbytes[4 * SpixelPos + 1] = Sbytes[4 * SpixelPos + 1];
                        Gbytes[4 * SpixelPos + 2] = Sbytes[4 * SpixelPos + 2];
                        Gbytes[4 * SpixelPos + 3] = Sbytes[4 * SpixelPos + 3];
                    } else {  // outside of Bezier curve
                        const int NDIV = 64;
                        const float dt = 1.0f / NDIV;
                        const glm::vec3 texelUV = glm::vec3(float(col) / (LODwidth - 1), 1.0f - float(row) / (LODheight - 1), 0.0);
                        int dist = INT_MAX;
                        for (int curve = 0; curve < numCurves; curve++) {
                            for (int div = 0; div < NDIV; div++) {
                                const float t = div * dt;

                                // change [-1, 1] space to [0, texWidth] space
                                glm::vec3 point = 0.5f * bezierCurve(curve, t) + glm::vec3(0.5f, 0.5f, 0.0f);

                                const int tmpDist = int(texWidth * glm::length(texelUV - point)) + 1;
                                dist = glm::min(dist, tmpDist);
                            }
                        }

                        // filter never intersects the curve, skip
                        if (dist > (MAXDIST - OVERLAP)) {
                            Gbytes[4 * SpixelPos + 0] = Sbytes[4 * SpixelPos + 0];
                            Gbytes[4 * SpixelPos + 1] = Sbytes[4 * SpixelPos + 1];
                            Gbytes[4 * SpixelPos + 2] = Sbytes[4 * SpixelPos + 2];
                            Gbytes[4 * SpixelPos + 3] = Sbytes[4 * SpixelPos + 3];
                            continue;
                        }

                        const int kernelIndex = dist - 1;
                        std::vector<std::vector<float>> outKernel = outKernels[kernelIndex];
                        const int outKernelSize = outKernelSizes[kernelIndex];

                        const int offset = outKernelSize / 2;

                        for (int RGB = 0; RGB < 3; RGB++) {
                            float weightedBytes = 0.0f;
                            float weightLoss = 1.0f;

                            for (int kernelY = -offset; kernelY <= offset; kernelY++) {
                                for (int kernelX = -offset; kernelX <= offset; kernelX++) {
                                    const int kernelRow = row + kernelY;
                                    const int kernelCol = col + kernelX;

                                    // operation for pixels out of original texture
                                    if (kernelRow < 0 ||
                                        kernelRow > LODheight - 1 ||
                                        kernelCol < 0 ||
                                        kernelCol > LODwidth - 1) {
                                        weightLoss -= outKernel[kernelY + offset][kernelX + offset];
                                        continue;
                                    }

                                    const int kernelSpixelPos = kernelRow * LODwidth + kernelCol;
                                    if (Sbytes[4 * kernelSpixelPos + RGB] == 0.0f) {
                                        weightLoss -= outKernel[kernelY + offset][kernelX + offset];
                                        continue;
                                    }
                                    weightedBytes += outKernel[kernelY + offset][kernelX + offset] * Sbytes[4 * kernelSpixelPos + RGB];
                                }
                            }
                            Gbytes[4 * SpixelPos + RGB] = (1.0f / weightLoss) * weightedBytes;
                        }
                        Gbytes[4 * SpixelPos + 3] = 1.0f;
                    }

                    // for following loops LOD > 0
                    memcpy(Pbytes, Gbytes, sizeof(float) * 4 * LODwidth * LODheight);
                } else {  // LOD > 0, simply apply gaussian filter
                    const int offset = kernelSize / 2;
                    for (int RGB = 0; RGB < 3; RGB++) {
                        // calculate weighted value using kernel for RGBA
                        float weightedBytes = 0.0f;
                        float weightLoss = 1.0f;
                        for (int kernelY = -offset; kernelY <= offset; kernelY++) {
                            for (int kernelX = -offset; kernelX <= offset; kernelX++) {
                                // operation for pixels beyond kernel border
                                if (col + kernelX < 0 ||
                                    col + kernelX > LODwidth - 1 ||
                                    row + kernelY < 0 ||
                                    row + kernelY > LODheight - 1) {
                                    weightLoss -= kernel[kernelY + offset][kernelX + offset];
                                    continue;
                                }

                                const int weightedPixelPos = SpixelPos + LODwidth * kernelY + kernelX;
                                if (Sbytes[4 * weightedPixelPos + RGB] == 0.0f) {
                                    weightLoss -= kernel[kernelY + offset][kernelX + offset];
                                } else {
                                    weightedBytes += kernel[kernelY + offset][kernelX + offset] * Sbytes[4 * weightedPixelPos + RGB];
                                }
                            }
                        }
                        Gbytes[4 * SpixelPos + RGB] = (1.0f / weightLoss) * weightedBytes;
                    }
                    Gbytes[4 * SpixelPos + 3] = 1.0f;
                }
            }
        }

        glTexSubImage2D(target, LOD, 0, 0, LODwidth, LODheight, GL_RGBA, GL_FLOAT, Gbytes);
        LODfactor *= 2;

        delete[] Sbytes;
        delete[] Gbytes;
    }

    glBindTexture(target, 0);

    delete[] Pbytes;
}

void BezierLight::compBernCoeffs() {
    int n = NUM_CPS_IN_CURVE - 1;
    for (int i = 0; i <= COEFF_DIV; i++) {
        double t = 1.0 / COEFF_DIV * i;

        bernCoeffs[i] = glm::vec4(bernstein(n, 0, t),
                                  bernstein(n, 1, t),
                                  bernstein(n, 2, t),
                                  bernstein(n, 3, t));
    }
}

void BezierLight::createBernCoeffTex() {
    // compute bernstein coeffs
    compBernCoeffs();

    glGenTextures(1, &bernCoeffTexId);

    GLenum target = GL_TEXTURE_1D;
    GLenum filter = GL_LINEAR;
    GLenum address = GL_CLAMP_TO_EDGE;

    glBindTexture(target, bernCoeffTexId);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, address);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, address);

    // upload
    glTexImage1D(target, 0, GL_RGBA32F, bernCoeffs.size(), 0, GL_RGBA, GL_FLOAT, bernCoeffs.data());

    glBindTexture(target, 0);
}

void BezierLight::drawBez(const Camera &camera) {
    glUseProgram(programId);

    GLuint location = glGetUniformLocation(programId, "u_cpsWorld");
    glUniform3fv(location, numPoints, glm::value_ptr(cpsWorld[0]));

    location = glGetUniformLocation(programId, "u_numCurves");
    glUniform1i(location, numCurves);

    location = glGetUniformLocation(programId, "u_isTwoSided");
    glUniform1i(location, isTwoSided);

    location = glGetUniformLocation(programId, "u_isBezTexed");
    glUniform1i(location, isBezTexed);

    if (isBezTexed) {
        location = glGetUniformLocation(programId, "u_texWidth");
        glUniform1i(location, texWidth);

        location = glGetUniformLocation(programId, "u_texHeight");
        glUniform1i(location, texHeight);

        location = glGetUniformLocation(programId, "u_marginSize");
        glUniform1i(location, marginSize);

        location = glGetUniformLocation(programId, "u_bezLightTex");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bezLightTexId);
        glUniform1i(location, 0);
    }

    //location = glGetUniformLocation(programId, "u_bernCoeffTex");
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_1D, bernCoeffTexId);
    //glUniform1i(location, 1);

    location = glGetUniformLocation(programId, "u_diffColor");
    glUniform3fv(location, 1, glm::value_ptr(diffColor));
    location = glGetUniformLocation(programId, "u_specColor");
    glUniform3fv(location, 1, glm::value_ptr(specColor));
    location = glGetUniformLocation(programId, "u_ambiColor");
    glUniform3fv(location, 1, glm::value_ptr(ambiColor));
    location = glGetUniformLocation(programId, "u_shininess");
    glUniform1f(location, shininess);

    glm::mat4 mMat, mvMat, mvpMat, normMat;
    mMat = modelMat;
    mvMat = camera.viewMat * mMat;
    mvpMat = camera.projMat * mvMat;
    normMat = glm::transpose(glm::inverse(mvMat));

    location = glGetUniformLocation(programId, "u_lightPos");
    glUniform3fv(location, 1, glm::value_ptr(center));
    location = glGetUniformLocation(programId, "u_lightLe");
    glUniform3fv(location, 1, glm::value_ptr(Le));
    location = glGetUniformLocation(programId, "u_lightMat");
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(camera.viewMat));
    location = glGetUniformLocation(programId, "u_mMat");
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(mMat));
    location = glGetUniformLocation(programId, "u_mvMat");
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(mvMat));
    location = glGetUniformLocation(programId, "u_mvpMat");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mvpMat));
    location = glGetUniformLocation(programId, "u_normMat");
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(normMat));

    if (textureId != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        location = glGetUniformLocation(programId, "u_isTextured");
        glUniform1i(location, true);
        location = glGetUniformLocation(programId, "u_texture");
        glUniform1i(location, 0);
    } else {
        location = glGetUniformLocation(programId, "u_isTextured");
        glUniform1i(location, 0);
    }

    glEnable(GL_STENCIL_TEST);
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_ALWAYS, 0, 1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
        glStencilMask(1);

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(ptsVaoId);
        glDrawArrays(GL_TRIANGLE_FAN, 0, samplePoints.size());
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilFunc(GL_EQUAL, 1, 1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glBindVertexArray(vaoId);
        glDrawElements(GL_TRIANGLES, bufferSize, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    glDisable(GL_STENCIL_TEST);

    glUseProgram(0);
}
