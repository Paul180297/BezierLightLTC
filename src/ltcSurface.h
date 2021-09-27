#pragma once

#include "bezierLight.h"
#include "render.h"

struct LtcSurface : public RenderObject {
    void initialize();
    void createLTCmatTex();
    void createLTCmagTex();
    void createRoughnessTex(const std::string &filename);

    void drawSurface(const Camera &camera, const BezierLight bezLight);

    float alpha;
    GLuint ltcMatTexId;
    GLuint ltcMagTexId;
    GLuint roughnessTexId;
    bool isRoughTexed;
};