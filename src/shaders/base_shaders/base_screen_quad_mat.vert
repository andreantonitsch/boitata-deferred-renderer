#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 color;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 vertexColor;

layout(push_constant) uniform matrices{
    mat4 model;
}push_constants;

void main() {
    gl_Position = push_constants.model * vec4(position, 1.0);
    vertexColor = color;
    outUV = inUV;
}