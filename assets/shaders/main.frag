#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct Material
{
	vec4 diffuse;
	int diffuseTextureId;
	float fuzziness;
	float refractionIndex;
	uint materialModel;
};

layout(binding = 1) readonly buffer materialArray { Material[] materials; };

layout(binding = 2) uniform UniformBufferObject {
    vec4 lightPos;
    vec4 lightColor;
};

layout(binding = 3) uniform sampler2D[] TextureSamplers;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in flat int fragMaterial;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = materials[fragMaterial].diffuse.xyz; 
	const int textureId = materials[fragMaterial].diffuseTextureId;
	if (textureId >= 0)
	{
		color = texture(TextureSamplers[textureId], fragTexCoord).rgb;
	}
	
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor.xyz;
	
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPos.xyz); 
	
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor.xyz;
	
	vec3 result = (ambient + diffuse) * color;
	outColor = vec4(result, 1.0f);
}