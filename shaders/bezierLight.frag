#version 410

#define PI 3.141592653589793
#define INV_PI 0.3183098861837907
#define MAX_N_POINTS 48
#define NUM_CPS_IN_CURVE 4

in vec3 f_vertPosWorld;
in vec2 f_texcoord;

out vec4 out_color;

uniform vec3 u_lightLe;
uniform vec3 u_cpsWorld[MAX_N_POINTS];

uniform int u_numCurves;
uniform int u_texWidth;
uniform int u_texHeight;
uniform int u_marginSize;

uniform bool u_isTwoSided;
uniform bool u_isBezTexed;

uniform sampler2D u_bezLightTex;

void correctUV(inout vec2 uv) {
    vec2 texSize = vec2(u_texWidth, u_texHeight);
    vec2 marginTexSize = texSize + 2 * vec2(u_marginSize);
    uv *= texSize / marginTexSize;
    uv += vec2(u_marginSize) / marginTexSize;
}

// ----------------------------------------------
// sRGB tone mapping
// ----------------------------------------------
const float gamma = 2.2;
vec3 toLinear(vec3 v) { return pow(v, vec3(gamma)); }
vec3 toSRGB(vec3 v)   { return pow(v, vec3(1.0 / gamma)); }

// ----------------------------------------------
// ACES tone mapper (borrwed from LTC program)
// https://eheitzresearch.wordpress.com/415-2/
// ----------------------------------------------
vec3 rrt_odt_fit(vec3 v)
{
    vec3 a = v*(         v + 0.0245786) - 0.000090537;
    vec3 b = v*(0.983729*v + 0.4329510) + 0.238081;
    return a/b;
}

mat3 mat3_from_rows(vec3 c0, vec3 c1, vec3 c2)
{
    mat3 m = mat3(c0, c1, c2);
    m = transpose(m);

    return m;
}

vec3 aces_fitted(vec3 color)
{
    mat3 ACES_INPUT_MAT = mat3_from_rows(
        vec3( 0.59719, 0.35458, 0.04823),
        vec3( 0.07600, 0.90834, 0.01566),
        vec3( 0.02840, 0.13383, 0.83777));

    mat3 ACES_OUTPUT_MAT = mat3_from_rows(
        vec3( 1.60475,-0.53108,-0.07367),
        vec3(-0.10208, 1.10813,-0.00605),
        vec3(-0.00327,-0.07276, 1.07602));

    color = ACES_INPUT_MAT * color;

    // Apply RRT and ODT
    color = rrt_odt_fit(color);

    color = ACES_OUTPUT_MAT * color;

    // Clamp to [0, 1]
    color = clamp(color, 0.0, 1.0);

    return color;
}

// ----------------------------------------------
// main
// ----------------------------------------------
void main(void) {
    vec3 color = vec3(1.0);
    
    if (u_isBezTexed) {
        vec2 uv = f_texcoord;
        correctUV(uv);
        color = textureLod(u_bezLightTex, uv, 0.0).rgb;
    }

    // color = aces_fitted(color);
    color = clamp(color, vec3(0.0), vec3(1.0));
    color = toSRGB(color);

    out_color = vec4(color, 1.0);
}
