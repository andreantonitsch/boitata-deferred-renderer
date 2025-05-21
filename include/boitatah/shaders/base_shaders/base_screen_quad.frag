#version 450

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 color;

layout(push_constant) uniform matrices{
    mat4 model;
}PushConstants;

void main() {

    vec4 albedo = vec4(UV, 1.0, 1.0);
}   