#version 450
layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 UV;

layout(push_constant) uniform matrices{
    mat4 model;
}PushConstants;

layout(set = 0, binding = 0) uniform Camera{
	mat4 vp;
	mat4 proj;
	mat4 view;
    vec3 viewPos;
    float aspect;
} camera_data;


void main() {
    gl_Position = camera_data.vp * PushConstants.model * vec4(position, 1.0);
    UV = uv;
}