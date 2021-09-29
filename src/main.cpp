#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "bezierLight.h"
#include "constants.h"
#include "ltcSurface.h"
#include "render.h"

static LtcSurface ltcFloor;
static BezierLight bezLight;
static Camera camera;
static bool isAnim = false;

static constexpr double Pi = 3.14159265358979;

#define SAVE_MOVIE 0

void initializeGL() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBlendEquation(GL_FUNC_ADD);

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClearStencil(0);

    // ltcFloor
    {
        ltcFloor.initialize();
        ltcFloor.loadOBJ(PLANE_OBJ);
        ltcFloor.buildShader(FLOORLTC_SHADER);
        ltcFloor.modelMat = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
        ltcFloor.diffColor = glm::vec3(1.0f);
        ltcFloor.specColor = glm::vec3(1.0f);

        ltcFloor.alpha = 0.01f;
        ltcFloor.createLTCmatTex();
        ltcFloor.createLTCmagTex();

        ltcFloor.isRoughTexed = true;
        if (ltcFloor.isRoughTexed) {
            ltcFloor.createRoughnessTex(ROUGHNESS_TEASER_PNG);
        }
    }

    // Bezier-curve light
    {
        bezLight.initialize();
        bezLight.loadOBJ(SMALLPLANE_OBJ);
        bezLight.buildShader(BEZLIGHT_SHADER);
        bezLight.Le = glm::vec3(1.0f);

        // create bezier curve points in normalized model space, then transform to world space
        bezLight.createCPSmodel(FOUR);
        bezLight.calcCPSworld();

        // create texture for bernstein coefficients, only for debugging
        //bezLight.createBernCoeffTex();

        // other settings
        bezLight.isBezTexed = true;
        if (bezLight.isBezTexed) {
            bezLight.createBezLightTex(GRADATION_PNG);
        }
    }

    // Camera parameters
    {
        camera.cameraPos = glm::vec3(0.0f, 6.0f, 32.5f);
        camera.cameraDir = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        camera.viewMat = glm::lookAt(camera.cameraPos, camera.cameraDir, camera.cameraUp);
        camera.projMat = glm::perspective(glm::radians(50.0f), float(WIN_WIDTH) / float(WIN_HEIGHT), 1.0f, 100.0f);
    }
}

void draw(bool isShowGui = true) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // bezierLight
    {
        GLuint programId = bezLight.programId;
        glUseProgram(programId);
        bezLight.drawBez(camera);
        glUseProgram(0);
    }

    // ltcFloor
    {
        GLuint programId = ltcFloor.programId;
        glUseProgram(programId);
        ltcFloor.drawSurface(camera, bezLight);
        glUseProgram(0);
    }

    // ImGui
    if (isShowGui) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Status");
        ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
        ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
        ImGui::Text("OpenGL: %s", glGetString(GL_VERSION));
        ImGui::Text("GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

        ImGui::Separator();

        static int s = 3;
        int prev_s = s;
        ImGui::Text("Scene:");
        const char *scene_chars[] = {"teardrop", "torch", "tripod", "rainbow", "cavity", "char", "camel"};
        ImGui::Combo(" ", &s, scene_chars, IM_ARRAYSIZE(scene_chars));

        static int a = 0;
        ImGui::Text("Roughness:");
        const char *alpha_chars[] = {"checker", "0.01", "0.1", "0.25", "0.4"};
        ImGui::Combo("  ", &a, alpha_chars, IM_ARRAYSIZE(alpha_chars));

        ImGui::Checkbox("Animate", &isAnim);
        ImGui::Checkbox("Light move", &bezLight.isMove);
        ImGui::Checkbox("Two-side", &bezLight.isTwoSided);

        static bool isVsync = true;
        ImGui::Checkbox("Vsync", &isVsync);
        glfwSwapInterval(isVsync ? 1 : 0);

        ImGui::End();

        switch (a) {
        case 0:
            ltcFloor.isRoughTexed = true;
            break;
        case 1:
            ltcFloor.isRoughTexed = false;
            ltcFloor.alpha = 0.01;
            break;
        case 2:
            ltcFloor.isRoughTexed = false;
            ltcFloor.alpha = 0.1;
            break;
        case 3:
            ltcFloor.isRoughTexed = false;
            ltcFloor.alpha = 0.25;
            break;
        case 4:
            ltcFloor.isRoughTexed = false;
            ltcFloor.alpha = 0.4;
            break;
        }

        // ONE, TWO, THREE, FOUR, CAVITYLEAF, CLIP, QUAD, CHAR
        if (s != prev_s) {
            switch (s) {
            case 0:
                bezLight.isBezTexed = false;
                bezLight.createCPSmodel(ONE);
                bezLight.calcCPSworld();
                break;
            case 1:
                bezLight.isBezTexed = false;
                bezLight.createCPSmodel(TWO);
                bezLight.calcCPSworld();
                break;
            case 2:
                bezLight.isBezTexed = false;
                bezLight.createCPSmodel(THREE);
                bezLight.calcCPSworld();
                break;
            case 3:
                bezLight.isBezTexed = true;
                bezLight.createCPSmodel(FOUR);
                bezLight.calcCPSworld();
                bezLight.createBezLightTex(GRADATION_PNG);
                break;
            case 4:
                bezLight.isBezTexed = false;
                bezLight.createCPSmodel(CAVITY);
                bezLight.calcCPSworld();
                break;
            case 5:
                bezLight.isBezTexed = false;
                bezLight.createCPSmodel(CHAR);
                bezLight.calcCPSworld();
                break;
            case 6:
                bezLight.isBezTexed = false;
                bezLight.createCPSmodel(CLIP);
                bezLight.calcCPSworld();
                break;
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void saveCurrentBuffer(GLFWwindow *window, const std::string &filename = "") {
    // Draw without GUI
    draw(false);

    // Capture image
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    auto bytes = std::make_unique<uint8_t[]>(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void *) bytes.get());

    // Invert vertically
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            const int iy = height - y - 1;
            for (int c = 0; c < 4; c++) {
                std::swap(bytes[(y * width + x) * 4 + c], bytes[(iy * width + x) * 4 + c]);
            }
        }
    }

    // Check file existence
    time_t now = time(0);
    tm *ts = localtime(&now);

    std::string outname = filename;
    if (filename.empty()) {
        char buf[128];
        strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S.png", ts);
        outname = std::string(buf);
    }

    // Save
    stbi_write_png(outname.c_str(), width, height, 4, bytes.get(), 0);
    printf("Buffer saved: %s\n", outname.c_str());
}

void update(GLFWwindow *window) {
    static int frameCount = 260;  // 180, 260, 320

#if SAVE_MOVIE
    {
        char filename[256];
        sprintf(filename, "%04d.png", frameCount);
        saveCurrentBuffer(window, filename);
    }
#endif

    camera.cameraPos.y = 1.0f;
    camera.cameraPos.x = 7.0f * sin(frameCount * Pi / 360 - 0.5f * Pi);
    camera.cameraPos.z = std::abs(7.0f * cos(frameCount * Pi / 360 - 0.5f * Pi));
    camera.viewMat = glm::lookAt(camera.cameraPos, camera.cameraDir, camera.cameraUp);

    // Move light
    if (bezLight.isMove) {
        bezLight.translate.y = 1.5f * std::cos(Pi * frameCount / 120.0f);
        bezLight.rotAngle.z = -frameCount * 0.5f;
        bezLight.calcCPSworld();
    }

    if (isAnim) {
        frameCount++;
    }
}

void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }

        if (key == GLFW_KEY_S && mods == GLFW_MOD_CONTROL) {
            saveCurrentBuffer(window);
        }
    }
}

void resize(GLFWwindow *window, int width, int height) {
    // Update window size
    glfwSetWindowSize(window, width, height);

    // Update viewport following actual window size
    int renderBufferWidth, renderBufferHeight;
    glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
    glViewport(0, 0, renderBufferWidth, renderBufferHeight);

    // Upcate projection matrix
    const float aspect = (float) renderBufferWidth / (float) renderBufferHeight;
    camera.projMat = glm::perspective(glm::radians(50.0f), aspect, 1.0f, 100.0f);
}

int main(int argc, char **argv) {
    if (glfwInit() == GL_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        return 1;
    }

#if !defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Window creation failed!\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fprintf(stderr, "Failed to load OpenGL 3.x/4.x libraries\n");
        fprintf(stderr, "Make sure at least OpenGL 4.3 is supported on your system\n");
        return 1;
    }
    printf("Load OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.Fonts->AddFontFromFileTTF("data/Roboto-Medium.ttf", 16.0f);
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Other setups
    initializeGL();
    glfwSetWindowSizeCallback(window, resize);
    glfwSetKeyCallback(window, keyboard);

    double totalTime = 0.0;
    uint32_t frameNum = 0;
    const uint32_t fpsUpdateSkip = 100;
    while (glfwWindowShouldClose(window) == GL_FALSE) {
        const double startTime = glfwGetTime();

        update(window);
        draw();

        frameNum++;
        if (frameNum % fpsUpdateSkip == 0) {
            const double timeSec = totalTime / fpsUpdateSkip;
            char title[256];
            sprintf(title, "%s: %.4f ms (%.2f fps)", WIN_TITLE, timeSec * 1000.0, 1.0 / timeSec);
            glfwSetWindowTitle(window, title);
            totalTime = 0.0;
            frameNum = 0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        totalTime += (glfwGetTime() - startTime);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
