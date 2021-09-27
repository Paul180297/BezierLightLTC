#include <glad/gl.h>
#include <stb_image.h>

#include "ltc2.inc"
#include "ltcSurface.h"

void LtcSurface::initialize() {
    alpha = 0.01;

    ltcMatTexId = -1;
    ltcMagTexId = -1;

    isRoughTexed = false;
    roughnessTexId = -1;
}

void LtcSurface::createLTCmatTex() {
    glGenTextures(1, &ltcMatTexId);

    GLenum target = GL_TEXTURE_2D;
    GLenum filter = GL_LINEAR;
    GLenum address = GL_CLAMP_TO_EDGE;

    glBindTexture(target, ltcMatTexId);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);

    glTexParameteri(target, GL_TEXTURE_WRAP_S, address);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, address);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    float data[size][size][4];

    for (int v = 0; v < size; ++v) {
        for (int u = 0; u < size; ++u) {
            // ltc2.inc
            float a = tabMinv[u * size + v % size][0] / (tabMinv[u * size + v % size][4]);
            float b = tabMinv[u * size + v % size][2] / (tabMinv[u * size + v % size][4]);
            float c = tabMinv[u * size + v % size][6] / (tabMinv[u * size + v % size][4]);
            float d = tabMinv[u * size + v % size][8] / (tabMinv[u * size + v % size][4]);

            data[u][v][0] = a;
            data[u][v][1] = b;
            data[u][v][2] = c;
            data[u][v][3] = d;
        }
    }

    // upload
    glTexImage2D(target, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, data);

    glBindTexture(target, 0);
}

void LtcSurface::createLTCmagTex() {
    glGenTextures(1, &ltcMagTexId);

    GLenum target = GL_TEXTURE_2D;
    GLenum filter = GL_LINEAR;
    GLenum address = GL_CLAMP_TO_EDGE;

    glBindTexture(target, ltcMagTexId);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);

    glTexParameteri(target, GL_TEXTURE_WRAP_S, address);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, address);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    float data[size * size];

    for (int u = 0; u < size; ++u) {
        for (int v = 0; v < size; ++v) {
            float a = tabAmplitude[u * size + v % size];

            data[u * size + v % size] = a;
        }
    }

    // upload
    glTexImage2D(target, 0, GL_R32F, size, size, 0, GL_RED, GL_FLOAT, data);

    glBindTexture(target, 0);
}

void LtcSurface::createRoughnessTex(const std::string &filename) {
    GLenum target = GL_TEXTURE_2D;
    GLenum filter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum address = GL_CLAMP_TO_EDGE;

    glGenTextures(1, &roughnessTexId);
    glBindTexture(target, roughnessTexId);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, address);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, address);

    int channels, texWidth, texHeight;
    unsigned char *bytes = stbi_load(filename.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);

    if (!bytes) {
        fprintf(stderr, "Failed to load image file: %s\n", filename.c_str());
        exit(1);
    }

    glTexImage2D(target, 0, GL_RGBA32F, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    glGenerateMipmap(target);

    glBindTexture(target, 0);
}

void LtcSurface::drawSurface(const Camera &camera, const BezierLight bezLight) {
    glUseProgram(programId);

    GLuint location = glGetUniformLocation(programId, "u_alpha");
    glUniform1f(location, alpha);

    location = glGetUniformLocation(programId, "u_isRoughTexed");
    glUniform1i(location, isRoughTexed);

    location = glGetUniformLocation(programId, "u_cameraPos");
    glUniform3fv(location, 1, glm::value_ptr(camera.cameraPos));

    location = glGetUniformLocation(programId, "u_cpsWorld");
    glUniform3fv(location, bezLight.numPoints, glm::value_ptr(bezLight.cpsWorld[0]));

    location = glGetUniformLocation(programId, "u_numCurves");
    glUniform1i(location, bezLight.numCurves);

    location = glGetUniformLocation(programId, "u_lightCenter");
    glUniform3fv(location, 1, glm::value_ptr(bezLight.center));

    location = glGetUniformLocation(programId, "u_isLightMove");
    glUniform1i(location, bezLight.isMove);

    location = glGetUniformLocation(programId, "u_isTwoSided");
    glUniform1i(location, bezLight.isTwoSided);

    location = glGetUniformLocation(programId, "u_isBezTexed");
    glUniform1i(location, bezLight.isBezTexed);

    location = glGetUniformLocation(programId, "u_ltcMatTex");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ltcMatTexId);
    glUniform1i(location, 0);

    location = glGetUniformLocation(programId, "u_ltcMagTex");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ltcMagTexId);
    glUniform1i(location, 1);

    if (isRoughTexed) {
        location = glGetUniformLocation(programId, "u_roughnessTex");
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, roughnessTexId);
        glUniform1i(location, 2);
    }

    if (bezLight.isBezTexed) {
        location = glGetUniformLocation(programId, "u_bezLightMmat");
        glUniformMatrix4fv(location, 1, false, glm::value_ptr(bezLight.modelMat));

        location = glGetUniformLocation(programId, "u_texWidth");
        glUniform1i(location, bezLight.texWidth);

        location = glGetUniformLocation(programId, "u_texHeight");
        glUniform1i(location, bezLight.texHeight);

        location = glGetUniformLocation(programId, "u_marginSize");
        glUniform1i(location, bezLight.marginSize);

        location = glGetUniformLocation(programId, "u_maxLOD");
        glUniform1i(location, bezLight.maxLOD);

        location = glGetUniformLocation(programId, "u_bezLightTex");
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, bezLight.bezLightTexId);
        glUniform1i(location, 3);
    }

    //location = glGetUniformLocation(programId, "u_bernCoeffTex");
    //glActiveTexture(GL_TEXTURE4);
    //glBindTexture(GL_TEXTURE_1D, bezLight.bernCoeffTexId);
    //glUniform1i(location, 4);

    draw(camera, bezLight.center, bezLight.Le);
}
