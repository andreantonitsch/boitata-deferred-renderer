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
    vec4 position;
    vec4 color;
    float intensity;
    uint data; //type/active/unused/unused
    uint index;
};

layout(set = 1, binding = 0) uniform LightData1{
    uint active_lights;
}light_metadata;

layout(set = 1, binding = 1) uniform LightData2{
    LightData[9999] light_points;
}light_array;

void main() {

    vec4 albedo = texture(color_tex , UV);
    vec4 position = texture(position_tex , UV);
    vec3 n = normalize(texture(normal_tex , UV).xyz);

    vec3 light_color = vec3(0.0).xyz;
    float intensity = 0.0f;

    for(uint i = 0; i < light_metadata.active_lights; i++){
        LightData light = light_array.light_points[i];
        vec3 p =  light.position.xyz - position.xyz;
        float r2 = dot(p,p);
        vec3 l = normalize(p);

        float ln = clamp(dot(l, n), 0.0, 1.0 );
        intensity += ln * (light.intensity/r2);
        light_color += light.color.xyz;
    }
    //intensity = min(2.0, intensity);
    color = vec4(albedo.xyz * intensity, 1.0);
}   