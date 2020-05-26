#version 460
#extension GL_EXT_ray_tracing : enable

struct RayPayload
{
	vec4 colorAndDistance; 
	vec4 scatterDirection; 
	uint randomSeed;
};

layout(location = 0) rayPayloadInEXT RayPayload ray;

void main() {
    ray.colorAndDistance = vec4(vec3(0.3), -1);
}