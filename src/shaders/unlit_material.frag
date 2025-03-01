#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 vertexColor;
//layout(location = 2) in vec3 normal;

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

layout(set = 1, binding = 0) uniform sampler2D color_tex;

void main() {
    color = texture(color_tex, UV) * vec4(vertexColor, 1.0);
}
