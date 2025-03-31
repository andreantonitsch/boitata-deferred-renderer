#version 450

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec4 color;

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

void main() {
    color = vec4(UV, 0.0, 1.0);
}
