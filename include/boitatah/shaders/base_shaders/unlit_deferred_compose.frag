#version 450

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 color;

layout(push_constant) uniform matrices{
    mat4 model;
}PushConstants;

layout(set = 0, binding = 0) uniform sampler2D color_tex;
layout(set = 0, binding = 1) uniform sampler2D position_tex;
layout(set = 0, binding = 2) uniform sampler2D normal_tex;

void main() {

    vec4 albedo = texture(color_tex , UV);
    vec4 position = texture(position_tex , UV);
    vec4 normal = texture(normal_tex , UV);
    color = albedo;
}   