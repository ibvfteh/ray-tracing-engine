#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 resultColor;

void main() {
    resultColor = vec3(0.3);
}