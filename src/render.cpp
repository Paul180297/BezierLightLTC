#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>

#include <glad/gl.h>
#include <stb_image.h>
#include <tiny_obj_loader.h>

#include "render.h"

Vertex::Vertex() :
    position(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), texcoord(0.0f, 0.0f) {
}

Vertex::Vertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &uv) :
    position(pos), normal(norm), texcoord(uv) {
}

void RenderObject::initialize() {
    programId = 0u;
    vaoId = 0u;
    vboId = 0u;
    iboId = 0u;
    textureId = 0u;
    bufferSize = 0;

    modelMat = glm::mat4(1.0f);
    ambiColor = glm::vec3(0.0f, 0.0f, 0.0f);
    diffColor = glm::vec3(0.0f, 0.0f, 0.0f);
    specColor = glm::vec3(0.0f, 0.0f, 0.0f);
}

void RenderObject::buildShader(const std::string &basename) {
    const std::string vertShaderFile = basename + ".vert";
    const std::string fragShaderFile = basename + ".frag";

    // Create shaders
    GLuint vertShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Load vertex shader source
    std::ifstream vertFileInput(vertShaderFile.c_str(), std::ios::in);
    if (!vertFileInput.is_open()) {
        fprintf(stderr, "Failed to load vertex shader: %s\n", vertShaderFile.c_str());
        exit(1);
    }
    std::istreambuf_iterator<char> vertDataBegin(vertFileInput);
    std::istreambuf_iterator<char> vertDataEnd;
    const std::string vertFileData(vertDataBegin, vertDataEnd);
    const char *vertShaderCode = vertFileData.c_str();

    // Load fragment shader source
    std::ifstream fragFileInput(fragShaderFile.c_str(), std::ios::in);
    if (!fragFileInput.is_open()) {
        fprintf(stderr, "Failed to load fragment shader: %s\n", fragShaderFile.c_str());
        exit(1);
    }
    std::istreambuf_iterator<char> fragDataBegin(fragFileInput);
    std::istreambuf_iterator<char> fragDataEnd;
    const std::string fragFileData(fragDataBegin, fragDataEnd);
    const char *fragShaderCode = fragFileData.c_str();

    // Compile
    GLint compileStatus;
    glShaderSource(vertShaderId, 1, &vertShaderCode, NULL);
    glCompileShader(vertShaderId);
    glGetShaderiv(vertShaderId, GL_COMPILE_STATUS, &compileStatus);
    printf("Compiling %s...\n", vertShaderFile.c_str());
    if (compileStatus == GL_FALSE) {
        fprintf(stderr, "Failed to compile vertex shader!\n");

        GLint logLength;
        glGetShaderiv(vertShaderId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLsizei length;
            char *errmsg = new char[logLength + 1];
            glGetShaderInfoLog(vertShaderId, logLength, &length, errmsg);

            std::cerr << errmsg << std::endl;
            fprintf(stderr, "%s", vertShaderCode);

            delete[] errmsg;
        }
    }

    glShaderSource(fragShaderId, 1, &fragShaderCode, NULL);
    glCompileShader(fragShaderId);
    glGetShaderiv(fragShaderId, GL_COMPILE_STATUS, &compileStatus);
    printf("Compiling %s...\n", fragShaderFile.c_str());
    if (compileStatus == GL_FALSE) {
        fprintf(stderr, "Failed to compile fragment shader!\n");

        GLint logLength;
        glGetShaderiv(fragShaderId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLsizei length;
            char *errmsg = new char[logLength + 1];
            glGetShaderInfoLog(fragShaderId, logLength, &length, errmsg);

            std::cerr << errmsg << std::endl;
            fprintf(stderr, "%s", vertShaderCode);

            delete[] errmsg;
        }
    }

    // Link to program
    programId = glCreateProgram();
    glAttachShader(programId, vertShaderId);
    glAttachShader(programId, fragShaderId);

    GLint linkState;
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
    if (linkState == GL_FALSE) {
        fprintf(stderr, "Failed to link shaders!\n");

        GLint logLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLsizei length;
            char *errmsg = new char[logLength + 1];
            glGetProgramInfoLog(programId, logLength, &length, errmsg);

            std::cerr << errmsg << std::endl;
            delete[] errmsg;
        }

        exit(1);
    }
}

void RenderObject::loadOBJ(const std::string &filename) {
    // Load OBJ file.
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
    if (!err.empty()) {
        std::cerr << "[WARNING] " << err << std::endl;
    }

    if (!success) {
        std::cerr << "Failed to load OBJ file: " << filename << std::endl;
        exit(1);
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    for (int s = 0; s < shapes.size(); s++) {
        const tinyobj::shape_t &shape = shapes[s];
        for (int i = 0; i < shape.mesh.indices.size(); i++) {
            const tinyobj::index_t &index = shapes[s].mesh.indices[i];

            Vertex vertex;
            if (index.vertex_index >= 0) {
                vertex.position = glm::vec3(
                    attrib.vertices[index.vertex_index * 3 + 0],
                    attrib.vertices[index.vertex_index * 3 + 1],
                    attrib.vertices[index.vertex_index * 3 + 2]);
            }

            if (index.normal_index >= 0) {
                vertex.normal = glm::vec3(
                    attrib.normals[index.normal_index * 3 + 0],
                    attrib.normals[index.normal_index * 3 + 1],
                    attrib.normals[index.normal_index * 3 + 2]);
            }

            if (index.texcoord_index >= 0) {
                vertex.texcoord = glm::vec2(
                    attrib.texcoords[index.texcoord_index * 2 + 0],
                    1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]);
            }

            indices.push_back((uint32_t) vertices.size());
            vertices.push_back(vertex);
        }
    }

    // Prepare VAO.
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                 vertices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, texcoord));

    glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
    bufferSize = (int)indices.size();

    glBindVertexArray(0);
}

void RenderObject::loadTexture(const std::string &filename) {
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    int texWidth, texHeight, channels;
    unsigned char *bytes = stbi_load(filename.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
    if (!bytes) {
        fprintf(stderr, "Failed to load image file: %s\n", filename.c_str());
        exit(1);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texWidth, texHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(bytes);
}

void RenderObject::draw(const Camera &camera, const glm::vec3 &lightPos, const glm::vec3 &lightLe) {
    glUseProgram(programId);

    GLuint location;
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
    glUniform3fv(location, 1, glm::value_ptr(lightPos));
    location = glGetUniformLocation(programId, "u_lightLe");
    glUniform3fv(location, 1, glm::value_ptr(lightLe));
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

    glBindVertexArray(vaoId);
    glDrawElements(GL_TRIANGLES, bufferSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}
