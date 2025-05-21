#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 color;


layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 vertexColor;
layout(location = 2) out vec4 vertexPosition;

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
    vertexPosition = PushConstants.model * vec4(position, 1.0);
    gl_Position = camera_data.vp * vertexPosition;

    vertexColor = color;
    outUV = inUV;
}