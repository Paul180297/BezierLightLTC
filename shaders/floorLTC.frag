#version 410

#define PI 3.141592653589793
#define INV_PI 0.3183098861837907
#define TWO_PI 6.283185307179586
#define INV_TWO_PI 0.15915494309189535
#define EPS 1.0e-5

#define MAX_N_POINTS 48
#define NUM_CPS_IN_CURVE 4 // 3rd-order Bezier curve
#define MAX_N_CURVES (MAX_N_POINTS / NUM_CPS_IN_CURVE)
#define NUM_INTERSECTION_MAX 3 // 3rd-order Bezier curve

in vec3 f_normalWorld;
in vec3 f_vertPosWorld;
in vec2 f_texcoord;

out vec4 out_color;

uniform mat4 u_bezLightMmat;

uniform float u_alpha;
uniform vec3 u_diffColor;
uniform vec3 u_specColor;

uniform vec3 u_cameraPos;

uniform vec3 u_lightLe;
uniform bool u_isLightMove;
uniform vec3 u_cpsWorld[MAX_N_POINTS];

uniform int u_numCurves;
uniform int u_texWidth;
uniform int u_texHeight;
uniform int u_marginSize;
uniform int u_maxLOD;

uniform bool u_isRoughTexed;
uniform bool u_isTwoSided;
uniform bool u_isBezTexed;

uniform sampler2D u_ltcMatTex;
uniform sampler2D u_ltcMagTex;
uniform sampler2D u_bezLightTex;
uniform sampler2D u_roughnessTex;

const float LUT_SIZE  = 64.0;
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS  = 0.5/LUT_SIZE;

// ----------------------------------------------
// array for color mapping
// ----------------------------------------------
const vec3 cmap_inferno[256] = vec3[256] (
    vec3(   3,   0,   0 ), vec3(   4,   0,   0 ), vec3(   6,   0,   0 ), vec3(   7,   0,   1 ),
    vec3(   9,   1,   1 ), vec3(  11,   1,   1 ), vec3(  14,   1,   2 ), vec3(  16,   2,   2 ),
    vec3(  18,   2,   3 ), vec3(  20,   3,   4 ), vec3(  22,   3,   4 ), vec3(  24,   4,   5 ),
    vec3(  27,   4,   6 ), vec3(  29,   5,   7 ), vec3(  31,   6,   8 ), vec3(  33,   6,   9 ),
    vec3(  35,   7,  10 ), vec3(  38,   7,  11 ), vec3(  40,   8,  13 ), vec3(  42,   8,  14 ),
    vec3(  45,   9,  15 ), vec3(  47,   9,  16 ), vec3(  50,  10,  18 ), vec3(  52,  10,  19 ),
    vec3(  54,  11,  20 ), vec3(  57,  11,  22 ), vec3(  59,  11,  23 ), vec3(  62,  11,  25 ),
    vec3(  64,  11,  26 ), vec3(  67,  12,  28 ), vec3(  69,  12,  29 ), vec3(  71,  12,  31 ),
    vec3(  74,  12,  32 ), vec3(  76,  11,  34 ), vec3(  78,  11,  36 ), vec3(  80,  11,  38 ),
    vec3(  82,  11,  39 ), vec3(  84,  11,  41 ), vec3(  86,  10,  43 ), vec3(  88,  10,  45 ),
    vec3(  90,  10,  46 ), vec3(  92,  10,  48 ), vec3(  93,   9,  50 ), vec3(  95,   9,  52 ),
    vec3(  96,   9,  53 ), vec3(  97,   9,  55 ), vec3(  98,   9,  57 ), vec3( 100,   9,  59 ),
    vec3( 101,   9,  60 ), vec3( 102,   9,  62 ), vec3( 102,   9,  64 ), vec3( 103,   9,  65 ),
    vec3( 104,  10,  67 ), vec3( 105,  10,  69 ), vec3( 105,  10,  70 ), vec3( 106,  11,  72 ),
    vec3( 106,  11,  74 ), vec3( 107,  12,  75 ), vec3( 107,  12,  77 ), vec3( 108,  13,  79 ),
    vec3( 108,  13,  80 ), vec3( 108,  14,  82 ), vec3( 109,  14,  83 ), vec3( 109,  15,  85 ),
    vec3( 109,  15,  87 ), vec3( 109,  16,  88 ), vec3( 109,  17,  90 ), vec3( 110,  17,  91 ),
    vec3( 110,  18,  93 ), vec3( 110,  18,  95 ), vec3( 110,  19,  96 ), vec3( 110,  20,  98 ),
    vec3( 110,  20,  99 ), vec3( 110,  21, 101 ), vec3( 110,  21, 102 ), vec3( 110,  22, 104 ),
    vec3( 110,  23, 106 ), vec3( 110,  23, 107 ), vec3( 110,  24, 109 ), vec3( 110,  24, 110 ),
    vec3( 110,  25, 112 ), vec3( 109,  25, 114 ), vec3( 109,  26, 115 ), vec3( 109,  27, 117 ),
    vec3( 109,  27, 118 ), vec3( 109,  28, 120 ), vec3( 109,  28, 122 ), vec3( 108,  29, 123 ),
    vec3( 108,  29, 125 ), vec3( 108,  30, 126 ), vec3( 107,  31, 128 ), vec3( 107,  31, 129 ),
    vec3( 107,  32, 131 ), vec3( 106,  32, 133 ), vec3( 106,  33, 134 ), vec3( 106,  33, 136 ),
    vec3( 105,  34, 137 ), vec3( 105,  34, 139 ), vec3( 105,  35, 141 ), vec3( 104,  36, 142 ),
    vec3( 104,  36, 144 ), vec3( 103,  37, 145 ), vec3( 103,  37, 147 ), vec3( 102,  38, 149 ),
    vec3( 102,  38, 150 ), vec3( 101,  39, 152 ), vec3( 100,  40, 153 ), vec3( 100,  40, 155 ),
    vec3(  99,  41, 156 ), vec3(  99,  41, 158 ), vec3(  98,  42, 160 ), vec3(  97,  43, 161 ),
    vec3(  97,  43, 163 ), vec3(  96,  44, 164 ), vec3(  95,  44, 166 ), vec3(  95,  45, 167 ),
    vec3(  94,  46, 169 ), vec3(  93,  46, 171 ), vec3(  92,  47, 172 ), vec3(  91,  48, 174 ),
    vec3(  91,  49, 175 ), vec3(  90,  49, 177 ), vec3(  89,  50, 178 ), vec3(  88,  51, 180 ),
    vec3(  87,  51, 181 ), vec3(  86,  52, 183 ), vec3(  86,  53, 184 ), vec3(  85,  54, 186 ),
    vec3(  84,  55, 187 ), vec3(  83,  55, 189 ), vec3(  82,  56, 190 ), vec3(  81,  57, 191 ),
    vec3(  80,  58, 193 ), vec3(  79,  59, 194 ), vec3(  78,  60, 196 ), vec3(  77,  61, 197 ),
    vec3(  76,  62, 199 ), vec3(  75,  62, 200 ), vec3(  74,  63, 201 ), vec3(  73,  64, 203 ),
    vec3(  72,  65, 204 ), vec3(  71,  66, 205 ), vec3(  70,  68, 207 ), vec3(  68,  69, 208 ),
    vec3(  67,  70, 209 ), vec3(  66,  71, 210 ), vec3(  65,  72, 212 ), vec3(  64,  73, 213 ),
    vec3(  63,  74, 214 ), vec3(  62,  75, 215 ), vec3(  61,  77, 217 ), vec3(  59,  78, 218 ),
    vec3(  58,  79, 219 ), vec3(  57,  80, 220 ), vec3(  56,  82, 221 ), vec3(  55,  83, 222 ),
    vec3(  54,  84, 223 ), vec3(  52,  86, 224 ), vec3(  51,  87, 226 ), vec3(  50,  88, 227 ),
    vec3(  49,  90, 228 ), vec3(  48,  91, 229 ), vec3(  46,  92, 230 ), vec3(  45,  94, 230 ),
    vec3(  44,  95, 231 ), vec3(  43,  97, 232 ), vec3(  42,  98, 233 ), vec3(  40, 100, 234 ),
    vec3(  39, 101, 235 ), vec3(  38, 103, 236 ), vec3(  37, 104, 237 ), vec3(  35, 106, 237 ),
    vec3(  34, 108, 238 ), vec3(  33, 109, 239 ), vec3(  31, 111, 240 ), vec3(  30, 112, 240 ),
    vec3(  29, 114, 241 ), vec3(  28, 116, 242 ), vec3(  26, 117, 242 ), vec3(  25, 119, 243 ),
    vec3(  24, 121, 243 ), vec3(  22, 122, 244 ), vec3(  21, 124, 245 ), vec3(  20, 126, 245 ),
    vec3(  18, 128, 246 ), vec3(  17, 129, 246 ), vec3(  16, 131, 247 ), vec3(  14, 133, 247 ),
    vec3(  13, 135, 248 ), vec3(  12, 136, 248 ), vec3(  11, 138, 248 ), vec3(   9, 140, 249 ),
    vec3(   8, 142, 249 ), vec3(   8, 144, 249 ), vec3(   7, 145, 250 ), vec3(   6, 147, 250 ),
    vec3(   6, 149, 250 ), vec3(   6, 151, 250 ), vec3(   6, 153, 251 ), vec3(   6, 155, 251 ),
    vec3(   6, 157, 251 ), vec3(   7, 158, 251 ), vec3(   7, 160, 251 ), vec3(   8, 162, 251 ),
    vec3(  10, 164, 251 ), vec3(  11, 166, 251 ), vec3(  13, 168, 251 ), vec3(  14, 170, 251 ),
    vec3(  16, 172, 251 ), vec3(  18, 174, 251 ), vec3(  20, 176, 251 ), vec3(  22, 177, 251 ),
    vec3(  24, 179, 251 ), vec3(  26, 181, 251 ), vec3(  28, 183, 251 ), vec3(  30, 185, 251 ),
    vec3(  33, 187, 250 ), vec3(  35, 189, 250 ), vec3(  37, 191, 250 ), vec3(  40, 193, 250 ),
    vec3(  42, 195, 249 ), vec3(  44, 197, 249 ), vec3(  47, 199, 249 ), vec3(  49, 201, 248 ),
    vec3(  52, 203, 248 ), vec3(  55, 205, 248 ), vec3(  58, 207, 247 ), vec3(  60, 209, 247 ),
    vec3(  63, 211, 246 ), vec3(  66, 213, 246 ), vec3(  69, 215, 245 ), vec3(  72, 217, 245 ),
    vec3(  75, 219, 244 ), vec3(  79, 220, 244 ), vec3(  82, 222, 243 ), vec3(  86, 224, 243 ),
    vec3(  89, 226, 243 ), vec3(  93, 228, 242 ), vec3(  96, 230, 242 ), vec3( 100, 232, 241 ),
    vec3( 104, 233, 241 ), vec3( 108, 235, 241 ), vec3( 112, 237, 241 ), vec3( 116, 238, 241 ),
    vec3( 121, 240, 241 ), vec3( 125, 242, 241 ), vec3( 129, 243, 242 ), vec3( 133, 244, 242 ),
    vec3( 137, 246, 243 ), vec3( 141, 247, 244 ), vec3( 145, 248, 245 ), vec3( 149, 250, 246 ),
    vec3( 153, 251, 247 ), vec3( 157, 252, 249 ), vec3( 160, 253, 250 ), vec3( 164, 254, 252 )
);

// ----------------------------------------------
// stack buffer for Douglas-Peucker integration
// ----------------------------------------------
vec2 DPstk[12];
int DPstkIndex = 0;

// ----------------------------------------------
// Bezier curve
// ----------------------------------------------
struct Bez {
    vec3 cps[NUM_CPS_IN_CURVE];
};

struct BezierLight {
    Bez bezs[MAX_N_POINTS / NUM_CPS_IN_CURVE];
};

void initBezierLight(out BezierLight bL) {
    for (int i = 0; i < u_numCurves; i++) {
        bL.bezs[i].cps[0] = u_cpsWorld[i * NUM_CPS_IN_CURVE + 0];
        bL.bezs[i].cps[1] = u_cpsWorld[i * NUM_CPS_IN_CURVE + 1];
        bL.bezs[i].cps[2] = u_cpsWorld[i * NUM_CPS_IN_CURVE + 2];
        bL.bezs[i].cps[3] = u_cpsWorld[i * NUM_CPS_IN_CURVE + 3];
    }
}

mat4 bernMat = transpose(mat4(1.0000,    0.0000,    0.0000,    0.0000,   // t = 1
                              0.2963,    0.4444,    0.2222,    0.0370,   // t = 0.6666
                              0.0370,    0.2222,    0.4444,    0.2963,   // t = 0.3333
                              0.0000,    0.0000,    0.0000,    1.0000)); // t = 0
mat4 invBernMat = transpose(mat4(1.0000,    0.0000,    0.0000,    0.0000,
                                -0.8333,    3.0000,   -1.5000,    0.3333,
                                 0.3333,   -1.5000,    3.0000,   -0.8333,
                                 0.0000,    0.0000,    0.0000,    1.0000));

vec3 bezierCurve(const Bez bez, float t) {
    // For cubic by de Casteljou's algorithm
    vec3 A = mix(bez.cps[0], bez.cps[1], t);
    vec3 B = mix(bez.cps[1], bez.cps[2], t);
    vec3 C = mix(bez.cps[2], bez.cps[3], t);

    A = mix(A, B, t);
    B = mix(B, C, t);

    return mix(A, B, t);
}

// ----------------------------------------------
// Bezier clipping
// ----------------------------------------------
void giftWrap(Bez tdBez, out int cvxLen, out vec3 cvx[MAX_N_POINTS]) {
    // Assume control points are on a plane.
    // Compute principal axes
    vec3 e0 = normalize(tdBez.cps[0] - tdBez.cps[1]);
    vec3 e1 = normalize(tdBez.cps[2] - tdBez.cps[1]);
    vec3 n = cross(e0, e1);
    vec3 tmp = vec3(0.0, 1.0, 0.0);
    if (abs(dot(n, tmp)) > 0.8) {
        tmp = vec3(0.0, 0.0, 1.0);
    }
    vec3 xAxis = cross(tmp, n);
    vec3 yAxis = cross(n, xAxis);

    vec2 pts[MAX_N_POINTS];
    for (int i = 0; i < NUM_CPS_IN_CURVE; i++) {
        pts[i] = vec2(dot(xAxis, tdBez.cps[i]), dot(yAxis, tdBez.cps[i]));
        cvx[i] = vec3(0.0, 0.0, 0.0);
    }

    // Find left most point
    int a = 0;
    for (int i = 1; i < NUM_CPS_IN_CURVE; i++) {
        if (pts[i].x < pts[a].x) {
            a = i;
        }
    }

    int idx[MAX_N_POINTS];
    int count = 0;
    for (int i = 0; i < NUM_CPS_IN_CURVE; i++) {
        idx[count] = a;
        count += 1;

        int b = 0;
        for (int i = 1; i < NUM_CPS_IN_CURVE; i++) {
            int c = i;
            if (b == a) {
                b = c;
            } else {
                vec2 ab = pts[b] - pts[a];
                vec2 ac = pts[c] - pts[a];
                float v = ab.x * ac.y - ab.y * ac.x;
                if (v > 0.0 || (v == 0.0 && length(ac) > length(ab))) {
                    b = c;
                }
            }
        }
        a = b;

        if (a == idx[0]) {
            break;
        }
    }

    for (int i = 0; i < count; i++) {
        cvx[i] = tdBez.cps[idx[i]];
    }
    cvxLen = count;
}

void bezierClipping(const Bez trBez, out int config, out int count, out float ts[NUM_INTERSECTION_MAX]) {
    // initialization
    count = 0;

    // check cases that do not need clipping
    int num = 0;
    for (int i = 0; i < NUM_CPS_IN_CURVE; i++) {
        num += trBez.cps[i].z < 0.0 ? 1 : 0;
    }

    if (num == 0) {
        config = 0;
        return;
    }
    else if (num == NUM_CPS_IN_CURVE) {
        config = 4;
        return;
    }

    /***** Bezier clipping *****/
    vec2 stk[MAX_N_POINTS];
    int stkIndex = 0;
    stk[stkIndex++] = vec2(0.0, 1.0); // initialization, push to stack

    while (stkIndex != 0) {
        vec2 tMinMax = stk[--stkIndex];
        float tMin = tMinMax.x;
        float tMax = tMinMax.y;

        bool isect = false;
        bool split = false;
        const int LOOP_NUM = 7;
        for (int loop = 0; loop < LOOP_NUM; loop++) {
            if (abs(tMax - tMin) < EPS) break;

            // 1. compute cps of bezier curve in td-space
            Bez tdBez;
            for (int i = 0; i < NUM_CPS_IN_CURVE; i++) {
                float t = tMin + (tMax - tMin) * (float(i) / float(NUM_CPS_IN_CURVE - 1));
                tdBez.cps[i].x = t;
                vec3 point = bezierCurve(trBez, t);
                tdBez.cps[i].y = point.z;
            }

            vec4 d = invBernMat * vec4(tdBez.cps[0].y, tdBez.cps[1].y, tdBez.cps[2].y, tdBez.cps[3].y);
            tdBez.cps[0].y = d.x;
            tdBez.cps[1].y = d.y;
            tdBez.cps[2].y = d.z;
            tdBez.cps[3].y = d.w;

            // 2. compute convex hull
            int cvxLen; // number of points generating the convex hull
            vec3 cvx[MAX_N_POINTS];
            giftWrap(tdBez, cvxLen, cvx);

            // 3. update tMin and tMax
            float new_tMin = 1.0e3;
            float new_tMax = -1.0e3;
            isect = false;
            for (int i = 0; i < cvxLen; i++) {
                int j = (i + 1) % cvxLen;
                if (sign(cvx[i].y) * sign(cvx[j].y) < 0.0) { // if the line connecting two points intersects the t-axis
                    float alpha = abs(cvx[i].y) / abs(cvx[i].y - cvx[j].y);
                    float t = mix(cvx[i].x, cvx[j].x, alpha);
                    new_tMin = min(t, new_tMin);
                    new_tMax = max(t, new_tMax);
                    isect = true;
                }
            }

            if (isect) { // if t was calculated
                if (abs(new_tMax - new_tMin) / abs(tMax - tMin) >= 0.5) {
                    // There are more than two intersections!
                    split = true;
                    break;
                }

                tMin = mix(tMin, new_tMin, 1.0) - EPS;
                tMax = mix(tMax, new_tMax, 1.0) + EPS;

                if (tMin > tMax) {
                    float tmp = tMin;
                    tMin = tMax;
                    tMax = tmp;
                }
            } else { // no intersection, break
                float avgY = (cvx[0].y + cvx[1].y + cvx[2].y + cvx[3].y) * 0.25;
                if (avgY < 0.0) { config = 3; }
                if (avgY > 0.0) { config = 1; }
                break;
            }
        }

        // found intersection
        if (split) {
            float tMid = 0.5 * (tMin + tMax);
            stk[stkIndex++] = vec2(tMin, tMid);
            stk[stkIndex++] = vec2(tMid, tMax);
        } else if (isect) {
            ts[count++] = 0.5 * (tMin + tMax);
        }
    }

    // if successfully clipped
    if (count >= 1 ) { config = 2; }
}

// ----------------------------------------------
// Algebraic clipping
// ----------------------------------------------
bool check01(const float t) {
    return 0.0 <= t && t <= 1.0;
}

float mad(float x, float a, float b) {
    return a * x + b;
}

bool almost(float a, float b) {
    return abs(a - b) < EPS;
}

vec3 sort3(vec3 v) {
    if (v.x > v.y) {
        float t = v.y;
        v.y = v.x;
        v.x = t;
    }
        
    if (v.y > v.z) {
        float t = v.z;
        v.z = v.y;
        v.y = t;
    }

    if (v.x > v.y) {
        float t = v.y;
        v.y = v.x;
        v.x = t;
    }
    return v;
}

void solveLinear(const float a, const float b,
                 inout int count, inout float ts[NUM_INTERSECTION_MAX]) {
    float t0 = -b / a;
    if (check01(t0)) { ts[count++] = t0; }
}

void solveQuadratic(const float a, const float b, const float c,
                    inout int count, inout float ts[NUM_INTERSECTION_MAX]) {
    float D = b * b - 4.0 * a * c;
    if (D > 0.0) { // two distinct real roots
        float sqrtD = sqrt(D);
        vec2 tt = -b + vec2(-1, 1) * sqrtD / (2.0 * a);
        if (check01(tt.y)) { ts[count++] = tt.y; }
        if (check01(tt.x)) { ts[count++] = tt.x; }
    }
}

// Solution for cubic equation based on the code in:
// http://momentsingraphics.de/CubicRoots.html
void solveCubic(const float a, const float b, const float c, const float d,
                inout int count, inout float ts[NUM_INTERSECTION_MAX]){
    // Normalize the polynomial
    vec4 Coefficient = vec4(d, c, b, a);
    Coefficient.xyz /= Coefficient.w;
    // Divide middle coefficients by three
    Coefficient.yz /= 3.0;
    // Compute the Hessian and the discrimant
    vec3 Delta = vec3(
        mad(-Coefficient.z, Coefficient.z, Coefficient.y),
        mad(-Coefficient.y, Coefficient.z, Coefficient.x),
        dot(vec2(Coefficient.z, -Coefficient.y),Coefficient.xy)
    );
    float Discriminant = dot(vec2(4.0f*Delta.x,-Delta.y),Delta.zy);

    // Compute coefficients of the depressed cubic 
    // (third is zero, fourth is one)
    vec2 Depressed = vec2(
        mad(-2.0 * Coefficient.z, Delta.x, Delta.y),
        Delta.x
    );

    if (Discriminant > 0.0) {
        // Take the cubic root of a normalized complex number
        float Theta = atan(sqrt(Discriminant),-Depressed.x)/3.0f;
        vec2 CubicRoot = vec2(cos(Theta), sin(Theta));
        // Compute the three roots, scale appropriately and 
        // revert the depression transform
        vec3 Root=vec3(
            CubicRoot.x,
            dot(vec2(-0.5, -0.5 * sqrt(3.0f)), CubicRoot),
            dot(vec2(-0.5,  0.5 * sqrt(3.0f)), CubicRoot)
        );
        vec3 tt = sort3(Root * 2.0 * sqrt(-Depressed.y) - Coefficient.z);
        if (check01(tt.z)) { ts[count++] = tt.z; }
        if (check01(tt.y)) { ts[count++] = tt.y; }
        if (check01(tt.x)) { ts[count++] = tt.x; }
    } else {
        vec2 tmp = 0.5 * (-Depressed.x + vec2(-1.0, 1.0) * sqrt(-Discriminant));
        vec2 pq = sign(tmp) * pow(abs(tmp), vec2(0.3333));
        float t0 = pq.x + pq.y - Coefficient.z;
        if (check01(t0)) { ts[count++] = t0; }
    }
}

void solveEquation(const float a, const float b, const float c, const float d,
                   inout int count, inout float ts[NUM_INTERSECTION_MAX]) {

    if (a != 0.0) {
        solveCubic(a, b, c, d, count, ts);
    } else if (b != 0.0) {
        solveQuadratic(b, c, d, count, ts);
    } else if (c != 0.0) {
        solveLinear(c, d, count, ts);
    }
}

void algebraicClipping(const Bez trBez, inout int config,
                       inout int count, inout float ts[NUM_INTERSECTION_MAX]) {
    // initialization
    count = 0;

    // check cases that do not need clipping
    int numCpsUnder = 0;
    for (int i = 0; i < NUM_CPS_IN_CURVE; i++) {
        numCpsUnder += (trBez.cps[i].z < 0.0) ? 1 : 0;
    }

    if (numCpsUnder == 0) {
        config = 0;
        return;
    }
    else if (numCpsUnder == NUM_CPS_IN_CURVE) {
        config = 4;
        return;
    }

    //*****algebraic clipping*****//
    float P0z = trBez.cps[0].z;
    float P1z = trBez.cps[1].z;
    float P2z = trBez.cps[2].z;
    float P3z = trBez.cps[3].z;

    // at^3 + bt^2 + ct + d = 0
    float a = -P0z + 3.0 * P1z - 3.0 * P2z + P3z;
    float b = 3.0 * (P0z - 2.0 * P1z + P2z);
    float c = 3.0 * (-P0z + P1z);
    float d = P0z;

    solveEquation(a, b, c, d, count, ts);
    if (count >= 1) { config = 2; }
    else { config = (bezierCurve(trBez, 0.5).z > 0.0) ? 1 : 3; } // if count == 0
}

// ----------------------------------------------
// line & curve integration in LTC
// ----------------------------------------------
float dist012(vec3 v0, vec3 v1, vec3 v2) {
    vec3 n = v2 - v0;
    return length(cross(v1 - v0, n)) / length(n);
}

mat3 calcCCmat(vec3 N, vec3 V, vec3 P, mat3 invM) {
    // construct orthonormal basis around N
    vec3 T1 = normalize(V - N * dot(V, N));
    vec3 T2 = cross(N, T1);

    mat3 tangentM = transpose(mat3(T1, T2, N));

    // matrix for rotating bezier light in "(T1, T2, N) basis"
    return invM * tangentM;
}

void transformToCC(const vec3 P, const mat3 CCmat, const Bez bez, inout Bez trBez) {
    // transform control points to CC space
    for (int i = 0; i < NUM_CPS_IN_CURVE; i++) {
        trBez.cps[i] = CCmat * (bez.cps[i] - P);
    }
}

float integrateEdge(const vec3 v0, const vec3 v1) {
    // project onto sphere
    float l0 = length(v0);
    float l1 = length(v1);
    float inv_l0l1 = 1.0 / (l0 * l1);

    float cosTheta = dot(v0, v1) * inv_l0l1;
    float absCosTheta = abs(cosTheta);

    float a = 5.42031 + (3.12829 + 0.0902326 * absCosTheta) * absCosTheta;
    float b = 3.45068 + (4.18814 + absCosTheta) * absCosTheta;
    float thetaOverSinTheta = a / b;

    if(cosTheta < 0.0) {
        thetaOverSinTheta = PI * inversesqrt(1.0 - cosTheta * cosTheta) - thetaOverSinTheta;
    }

    return thetaOverSinTheta * cross(v0, v1).z * inv_l0l1;
}

float DPintegration(const Bez trBez, const float tStart, const float tEnd, const int div, const float thres, inout int edgeNum) {
    float res = 0.0;
    
    // if line
    vec3 v01 = trBez.cps[1] - trBez.cps[0];
    vec3 v02 = trBez.cps[2] - trBez.cps[0];
    vec3 v03 = trBez.cps[3] - trBez.cps[0];
    if (abs(dot(v01, v02)) < EPS && abs(dot(v01, v03)) < EPS) {
        vec3 v0 = bezierCurve(trBez, tStart);
        vec3 v3 = bezierCurve(trBez, tEnd);
        res = integrateEdge(v0, v3);
        return res;
    }

    // initialization
    float tRange = tEnd - tStart;
    float interval = tRange / div;
    for (int i = div; i > 0; i--) {
        // in descending order
        int j = i - 1;
        float tMin = interval * j + tStart;
        float tMax = interval * i + tStart;
        DPstk[DPstkIndex++] = vec2(tMin, tMax);
    }

    while (DPstkIndex != 0) {
        vec2 tmp = DPstk[--DPstkIndex];
        float tMin = tmp.x;
        float tMax = tmp.y;
        float tMid = 0.5 * (tMin + tMax);

        // compute intensity of triangle
        vec3 v0 = bezierCurve(trBez, tMin);
        vec3 v1 = bezierCurve(trBez, tMid);
        vec3 v2 = bezierCurve(trBez, tMax);

        float I01z = integrateEdge(v0, v1);
        float I12z = integrateEdge(v1, v2);
        float I20z = integrateEdge(v2, v0);

        float Iz = I01z + I12z + I20z;
        float relD = dist012(v0, v1, v2) / length(v2 - v0);
        
        if (abs(Iz) >= thres && relD > 0.01) { // if A is larger than threshold, add tMid to stack and repeat the loop
            DPstk[DPstkIndex++] = vec2(tMid, tMax);
            DPstk[DPstkIndex++] = vec2(tMin, tMid);
        } else {
            res += I01z + I12z;
            edgeNum += 2;
        }
    }

    return res;
}

// ----------------------------------------------
// diffuse & specular reflectance evaluation
// ----------------------------------------------
float evaluateLTCspec(vec3 P, const float alpha, const int nDiv, mat3 specCCmat, const BezierLight bezLight, bool twoSided, inout int edgeNum) {
    // integrate each curve
    float spec = 0.0;
    float thres = 0.1 * alpha * alpha; // alpha-based threshold

    bool hasBegin = false;
    vec3 vBegin = vec3(0.0);
    bool hasEnd = false;
    vec3 vEnd = vec3(0.0);

    BezierLight trBezLight;
    for (int curve = 0; curve < u_numCurves; curve++) {
        int configs; // array for detecting integration configs
        int counts; // number of intersections in each curve
        float ts[NUM_INTERSECTION_MAX]; // 2D array that stores all intesrctions
        transformToCC(P, specCCmat, bezLight.bezs[curve], trBezLight.bezs[curve]);

        // clipping methods, use either of these
        //bezierClipping(trBezLight.bezs[curve], configs[curve], counts[curve], ts[curve]);
        algebraicClipping(trBezLight.bezs[curve], configs, counts, ts);

        if (configs <= 1) {
            // 0: all cps above surface, integrate all
            // 1: entire curve above surface, integrate all
            spec += DPintegration(trBezLight.bezs[curve], 0.0, 1.0, nDiv, thres, edgeNum);
        } else if (configs >= 3) {
            // 3: entire curve below surface, no integration
            // 4: all cps below surface, no integration
            // do nothing
        } else {
            // 2: curve intersects with surface
            float t0, t1, t2;
            switch(counts) {
                case 0:
                    break; // do nothing

                case 1:
                    t0 = ts[0];
                    if (bezierCurve(trBezLight.bezs[curve], 0.5 * t0).z > 0.0) {
                        // start point is above surface
                        spec += DPintegration(trBezLight.bezs[curve], 0.0, t0, nDiv / 2, thres, edgeNum);
                        vEnd = bezierCurve(trBezLight.bezs[curve], t0);
                        hasEnd = true;
                    } else {
                        // end point is above surface
                        spec += DPintegration(trBezLight.bezs[curve], t0, 1.0, nDiv / 2, thres, edgeNum);
                        if (hasEnd) {
                            vec3 v0 = bezierCurve(trBezLight.bezs[curve], t0);
                            spec += integrateEdge(vEnd, v0);
                            vEnd = vec3(0.0);
                            hasEnd = false;
                        } else {
                            vBegin = bezierCurve(trBezLight.bezs[curve], t0);
                            hasBegin = true;
                        }
                    }
                    break;

                case 2:
                    // flip order: "ts[curve][1] < ts[curve][0]" -> "t0 < t1"
                    t0 = ts[1];
                    t1 = ts[0];
                    if (bezierCurve(trBezLight.bezs[curve], 0.5 * (t0 + t1)).z > 0.0) { // if mid point is above surface
                        // integrate t0 -> t1
                        spec += DPintegration(trBezLight.bezs[curve], t0, t1, nDiv / 2, thres, edgeNum);

                        if (hasEnd) {
                            vec3 v0 = bezierCurve(trBezLight.bezs[curve], t0);
                            spec += integrateEdge(vEnd, v0);
                            vEnd = vec3(0.0);
                            hasEnd = false;
                        } else {
                            vBegin = bezierCurve(trBezLight.bezs[curve], t0);
                            hasBegin = true;
                        }

                        vEnd = bezierCurve(trBezLight.bezs[curve], t1);
                        hasEnd = true;
                    } else { // if mid point is below surface
                        // integrate 0.0 -> t0
                        spec += DPintegration(trBezLight.bezs[curve], 0.0, t0, nDiv / 2, thres, edgeNum);

                        // integrate edge t0 -> t1 (connect t0 and t1)
                        vec3 v0 = bezierCurve(trBezLight.bezs[curve], t0);
                        vec3 v1 = bezierCurve(trBezLight.bezs[curve], t1);
                        spec += integrateEdge(v0, v1);

                        // integrate t1 -> 1.0
                        spec += DPintegration(trBezLight.bezs[curve], t1, 1.0, nDiv / 2, thres, edgeNum);
                    }
                    break;

                case 3: // combination of case1 and case2
                    // flip order: "ts[curve][2] < ts[curve][1] < ts[curve][0]" -> "t0 < t1 < t2"
                    t0 = ts[2];
                    t1 = ts[1];
                    t2 = ts[0];
                    if (bezierCurve(trBezLight.bezs[curve], 0.5 * (t0 + t1)).z > 0.0) { // if 0.5(t0 + t1) is above surface
                        // integrate t0 -> t1
                        spec += DPintegration(trBezLight.bezs[curve], t0, t1, nDiv / 2, thres, edgeNum);

                        // integrate edge t1 -> t2 (connect t1 and t2)
                        vec3 v1 = bezierCurve(trBezLight.bezs[curve], t1);
                        vec3 v2 = bezierCurve(trBezLight.bezs[curve], t2);
                        spec += integrateEdge(v1, v2);

                        // integrate t2 -> 1.0
                        spec += DPintegration(trBezLight.bezs[curve], t2, 1.0, nDiv / 2, thres, edgeNum);

                        vec3 v0 = bezierCurve(trBezLight.bezs[curve], t0);
                        if (hasEnd) {
                            spec += integrateEdge(vEnd, v0);
                            vEnd = vec3(0.0);
                            hasEnd = false;
                        } else {
                            vBegin = v0;
                            hasBegin = true;
                        }
                    } else {
                        // integrate 0.0 -> t0
                        spec += DPintegration(trBezLight.bezs[curve], 0.0, t0, nDiv / 2, thres, edgeNum);

                        // integrate edge t0 -> t1 (connect t0 and t1)
                        vec3 v0 = bezierCurve(trBezLight.bezs[curve], t0);
                        vec3 v1 = bezierCurve(trBezLight.bezs[curve], t1);
                        spec += integrateEdge(v0, v1);

                        // integrate t1 -> t2
                        spec += DPintegration(trBezLight.bezs[curve], t1, t2, nDiv / 2, thres, edgeNum);

                        vEnd = bezierCurve(trBezLight.bezs[curve], t2);
                        hasEnd = true;
                    }
                    break;
            }
        }
    }

    if (hasBegin && hasEnd) {
        spec += integrateEdge(vEnd, vBegin);
    }

    return u_isTwoSided ? abs(spec) : max(0.0, spec);
}

vec3 surfaceIntersection(const vec3 v0, const vec3 v1) {
    vec3 dir = v1 - v0;
    float invdir_z = 1.0 / (abs(dir.z) + EPS) * sign(dir.z);    
    vec3 res = v0 - dir * (v0.z * invdir_z);
    return res;
}

float evaluateLTCspec_simple(vec3 P, const int nDiv, mat3 specCCmat, const BezierLight bezLight, bool twoSided) {
    BezierLight trBezLight;
    for (int curve = 0; curve < u_numCurves; curve++) {
        transformToCC(P, specCCmat, bezLight.bezs[curve], trBezLight.bezs[curve]);
    }

    float spec = 0.0;
    bool hasBegin = false;
    vec3 vBegin = vec3(0.0);
    bool hasEnd = false;
    vec3 vEnd = vec3(0.0);
    for (int curve = 0; curve < u_numCurves; curve++) {
        float dt = 1.0 / nDiv;

        for (int div = 0; div < nDiv; div++) {
            float t0 = div * dt;
            float t1 = (div + 1) * dt;

            vec3 v0 = bezierCurve(trBezLight.bezs[curve], t0);
            vec3 v1 = bezierCurve(trBezLight.bezs[curve], t1);

            int config;
            if (v0.z > 0.0 && v1.z > 0.0) { config = 0; } // if the whole edge is above surface
            else if (v0.z > 0.0 && v1.z < 0.0) { config = 1; } // if v0 is above surface
            else if (v0.z < 0.0 && v1.z > 0.0) { config = 2; } // if v1 is above surface
            else { config = 3; } // if the whole edge is below surface
            
            switch (config) {
                case 0: // integrate whole edge
                    spec += integrateEdge(v0, v1);
                    break;

                case 1: // v1 is on the surface
                    v1 = surfaceIntersection(v0, v1);
                    spec += integrateEdge(v0, v1);
                    vEnd = v1;
                    hasEnd = true;
                    break;

                case 2: // v0 is on the surface
                    v0 = surfaceIntersection(v0, v1);
                    spec += integrateEdge(v0, v1);
                    if (hasEnd) {
                        spec += integrateEdge(vEnd, v0);
                        vEnd = vec3(0.0);
                        hasEnd = false;
                    } else {
                        vBegin = v0;
                        hasBegin = true;
                    }
                    break;

                case 3:
                    // do nothing
                    break;
            }
        }
    }

    if (hasBegin && hasEnd) {
        spec += integrateEdge(vEnd, vBegin);
    }

    return u_isTwoSided ? abs(spec) : max(0.0, spec);
}

float evaluateLTCdiff(vec3 P, mat3 diffCCmat, const BezierLight bL, bool twoSided) {
    float diff = 0.0;
    int dummy = 0;
    for (int curve = 0; curve < u_numCurves; curve++) {
        Bez trBez;
        transformToCC(P, diffCCmat, bL.bezs[curve], trBez);

        const int DIV = 4;
        const float thres = 1000.0;
        diff += DPintegration(trBez, 0.0, 1.0, DIV, thres, dummy);
    }
    return u_isTwoSided ? abs(diff) : max(0.0, diff);
}

// UV calculation
void correctUV(inout vec2 uv) {
    vec2 texSize = vec2(u_texWidth, u_texHeight);
    vec2 marginTexSize = texSize + 2 * vec2(u_marginSize);
    uv *= texSize / marginTexSize;
    uv += vec2(u_marginSize) / marginTexSize;
}

void calcUVandLOD(vec3 P, const mat3 CCmat, const float alpha, out vec2 texcoord, out float LOD) {
    // Bezier curve is defined in [-1, 1] space
    vec3 leftDown  = CCmat * ((u_bezLightMmat * vec4(-1.0, -1.0, 0.0, 1.0)).xyz - P);
    vec3 rightDown = CCmat * ((u_bezLightMmat * vec4( 1.0, -1.0, 0.0, 1.0)).xyz - P);
    vec3 rightUp   = CCmat * ((u_bezLightMmat * vec4( 1.0,  1.0, 0.0, 1.0)).xyz - P);
    vec3 leftUp    = CCmat * ((u_bezLightMmat * vec4(-1.0,  1.0, 0.0, 1.0)).xyz - P);

    vec3 invDirX = normalize(rightDown - leftDown);
    vec3 invDirY = normalize(leftUp - leftDown);

    vec3 polygonN = cross(invDirX, invDirY);
    vec3 x0 = vec3(0.0);

    vec3 dir = polygonN;

    float t = -dot(polygonN, x0 - leftDown) / dot(polygonN, dir);
    vec3 intersectPoint = x0 + t * dir;

    vec3 intersectPointLD = intersectPoint - leftDown;
    vec3 intersectPointRU = intersectPoint - rightUp;

    vec3 N01 = normalize(cross(polygonN, rightDown - leftDown));
    vec3 N03 = normalize(cross(leftUp - leftDown, polygonN));
    vec3 N12 = normalize(cross(polygonN, rightUp - rightDown));
    vec3 N32 = normalize(cross(rightUp - leftUp, polygonN));

    float u = dot(intersectPointLD, N03) / (dot(intersectPointLD, N03) + dot(intersectPointRU, N12));
    float v = dot(intersectPointLD, N01) / (dot(intersectPointLD, N01) + dot(intersectPointRU, N32));

    texcoord = vec2(u, 1.0 - v);
    correctUV(texcoord);

    // LOD calculation
    vec3 center = 0.25 * (leftDown + rightDown + rightUp + leftUp);
    float r = length(center); // length between shading point and light center in CC
    float A = length(cross(rightDown - leftDown, leftUp - leftDown));
    float sigma = 4.0 * r * r * inversesqrt(2.0 * A); // 4.0 * r * r
    LOD = (sigma + u_maxLOD) * alpha;
}

// ----------------------------------------------
// tone mappers
// ----------------------------------------------
const float gamma = 2.2;
vec3 toLinear(in vec3 v) { return pow(clamp(v, vec3(0.0), vec3(1.0)), vec3(gamma)); }
vec3 toSRGB(in vec3 v)   { return pow(clamp(v, vec3(0.0), vec3(1.0)), vec3(1.0 / gamma)); }

vec3 rrt_odt_fit(vec3 v) {
    vec3 a = v*(         v + 0.0245786) - 0.000090537;
    vec3 b = v*(0.983729*v + 0.4329510) + 0.238081;
    return a/b;
}

mat3 mat3_from_rows(vec3 c0, vec3 c1, vec3 c2) {
    mat3 m = mat3(c0, c1, c2);
    m = transpose(m);

    return m;
}

vec3 aces_fitted(vec3 color) {
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
    BezierLight bL;
    initBezierLight(bL);

    vec3 diffColor = toLinear(vec3(u_diffColor));
    vec3 specColor = toLinear(vec3(u_specColor));

    vec3 color = vec3(0.0);

    vec3 P = f_vertPosWorld;
    vec3 N = normalize(f_normalWorld);
    vec3 V = normalize(vec3(u_cameraPos) - P);

    // ltc2.inc uv calculation
    float alpha = u_alpha;
    if (u_isRoughTexed) { alpha = texture(u_roughnessTex, f_texcoord).x; }
    alpha = clamp(alpha, 0.01, 1.0);
    float ndotv = clamp(dot(N, V), 0.0, 1.0);

    vec2 uv = vec2(alpha, sqrt(1.0 - ndotv));
    uv = uv * LUT_SCALE + LUT_BIAS;
    vec4 t = texture(u_ltcMatTex, vec2(uv));

    //ltc2.inc
    mat3 invM = mat3(
        vec3(t.x, 0, t.y),
        vec3(  0,  1,  0),
        vec3(t.z, 0, t.w)
    );

    int edgeNum = 0;
    mat3 specCCmat = calcCCmat(N, V, P, invM);
    float spec = evaluateLTCspec(P, alpha, 4, specCCmat, bL, u_isTwoSided, edgeNum) * texture(u_ltcMagTex, uv).x;

    // Use only for experiment
    // Approximate contour curve by simple uniform polygon,
    // clipping performed by checking intersection for every segment of the curve
    //float spec = evaluateLTCspec_simple(P, 20, specCCmat, bL, u_isTwoSided) * texture(u_ltcMagTex, uv).x;

    mat3 diffCCmat = calcCCmat(N, V, P, mat3(1.0));
    float diff = evaluateLTCspec(P, 1.0, 4, diffCCmat, bL, u_isTwoSided, edgeNum);
    //float diff = evaluateLTCdiff(P, diffCCmat, bL, u_isTwoSided);  // assume light does not cross with the ground
    //float diff = evaluateLTCspec_simple(P, 4, diffCCmat, bL, u_isTwoSided);

    vec3 specLightColor = vec3(1.0);
    vec3 diffLightColor = vec3(1.0);
    if (u_isBezTexed) {
        vec2 texcoord = vec2(0.0);
        float LOD = 0;
        calcUVandLOD(P, specCCmat, alpha, texcoord, LOD);
        specLightColor = textureLod(u_bezLightTex, texcoord, LOD).rgb;
        diffLightColor = textureLod(u_bezLightTex, vec2(0.5), u_maxLOD).rgb;
    }
    color = u_lightLe * (specLightColor * spec * specColor + diffLightColor * diff * diffColor) * INV_TWO_PI;

    color = clamp(color, vec3(0.0), vec3(1.0));
    color = toSRGB(color);

    out_color = vec4(color, 1.0);

    // color mapping, uncomment if needed
    //const int colorIndex = int(clamp(edgeNum / 256.0 * 256, 0, 255));
    //out_color = vec4(vec3(cmap_inferno[colorIndex].zyx) / 256.0, 1.0);
}
