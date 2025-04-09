#version 450

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 color;

layout(push_constant) uniform matrices{
    mat4 model;
}PushConstants;

layout(set = 0, binding = 0) uniform sampler2D color_tex;
layout(set = 0, binding = 1) uniform sampler2D position_tex;
layout(set = 0, binding = 2) uniform sampler2D normal_tex;

struct LightData{
    vec3 position;
    vec3 color;
    float intensity;
    vec4 data; //type/active/unused/unused
};

layout(set = 1, binding = 0) uniform LightData1{
    uint active_lights;
};

layout(set = 1, binding = 1) uniform LightData2{\
    LightData[9999] light_points;
};

void main() {

    vec4 albedo = texture(color_tex , UV);
    vec4 position = texture(position_tex , UV);
    vec4 normal = texture(normal_tex , UV);






    color = albedo;
}   