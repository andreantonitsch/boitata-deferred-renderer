#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoord;

layout(binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

void main(){
	// using vertex indexing and hardcoded arrays.
	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	//fragColor = colors[gl_VertexIndex];

	vec4 position = ubo.proj * ubo.view * ubo.model  * vec4(inPosition, 1.0);
	gl_Position = position;
	fragColor = inColor;
	texCoord = inTexCoord;
}