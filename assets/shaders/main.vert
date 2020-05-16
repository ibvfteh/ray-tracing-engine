#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConsts {
    vec4 clipPlane;
    float ref;
} pushConsts;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 underView;
    mat4 proj;
    vec4 cameraPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in int inMaterial;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out int fragMaterial;

void main() {
    
    vec4 worldPosition = ubo.model * vec4(inPosition, 1.0);
    gl_ClipDistance[0] = dot(worldPosition, pushConsts.clipPlane);
    if (pushConsts.ref > 0.5)
    {
        gl_Position = ubo.proj * ubo.underView * worldPosition;
    }
    else
    { 
        gl_Position = ubo.proj * ubo.view * worldPosition;
    }
    fragPosition = (ubo.model * vec4(inPosition, 1.0)).xyz;
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
    fragMaterial = inMaterial;
}