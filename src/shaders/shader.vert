#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform matrices{
    mat4 model;
    mat4 vp;
}PushConstants;

void main() {
    gl_Position = PushConstants.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}
