#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 UV;
//layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform matrices{
    mat4 model;
}PushConstants;

layout(set = 0, binding = 0) uniform  Camera{
	mat4 vp;
	mat4 proj;
	mat4 view;
    vec3 viewPos;
    float aspect;
} camera_data;

layout(set = 0, binding = 0) uniform sampler2D color_tex;

void main() {
    outColor = vec4(fragColor, 1.0);
    //outColor = vec4(1.0, 0.0, 0.0, 0.0);
}
