#version 450

layout(location = 0) in vec2 UV;
layout(location = 1) in vec4 vertexPosition;
layout(location = 2) in vec4 vertexNormal;
layout(location = 3) in vec3 vertexColor;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 position;
layout(location = 2) out vec4 normal;

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
    normal = vec4(normalize(vertexNormal.xyz), 1.0);
    position = vertexPosition;
}