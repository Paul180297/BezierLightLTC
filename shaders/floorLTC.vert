#version 410

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

out vec3 f_normalWorld;
out vec3 f_vertPosWorld;
out vec2 f_texcoord;

uniform mat4 u_mMat;
uniform mat4 u_mvMat;
uniform mat4 u_mvpMat;
uniform mat4 u_normMat;

void main(){
    gl_Position = u_mvpMat * vec4(in_position, 1.0);
    f_normalWorld = (u_mMat * vec4(in_normal, 0.0)).xyz;
    f_vertPosWorld = (u_mMat * vec4(in_position, 1.0)).xyz;
    f_texcoord = vec2(in_texcoord.x, in_texcoord.y);
}
