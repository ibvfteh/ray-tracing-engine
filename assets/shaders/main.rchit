#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 resultColor;

const uint MaterialLambertian = 0;

struct Material
{
	vec4  Diffuse;
	int   DiffuseTextureId;
	float Fuzziness;
	float RefractionIndex;
	uint  MaterialModel;
};

struct Vertex
{
  vec3 position;
  vec3 normal;
  vec2 texCoord;
  int  materialIndex;
};

layout(binding = 3) readonly buffer VertexArray { float Vertices[]; };
layout(binding = 4) readonly buffer IndexArray { uint Indices[]; };
layout(binding = 5) readonly buffer MaterialArray { Material[] Materials; };
layout(binding = 6) readonly buffer OffsetArray { uvec2[] Offsets; };
layout(binding = 7) uniform sampler2D[] TextureSamplers;

hitAttributeEXT vec2 hitAttribs;

Vertex UnpackVertex(uint index)
{
	const uint vertexSize = 9;
	const uint offset = index * vertexSize;
	
	Vertex v;
	
	v.position = vec3(Vertices[offset + 0], Vertices[offset + 1], Vertices[offset + 2]);
	v.normal = vec3(Vertices[offset + 3], Vertices[offset + 4], Vertices[offset + 5]);
	v.texCoord = vec2(Vertices[offset + 6], Vertices[offset + 7]);
	v.materialIndex = floatBitsToInt(Vertices[offset + 8]);

	return v;
}

vec2 Mix(vec2 a, vec2 b, vec2 c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 Mix(vec3 a, vec3 b, vec3 c, vec3 barycentrics) 
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

void main() {    
	// Get the material.
	const uvec2 offsets = Offsets[gl_InstanceCustomIndexEXT];
	const uint indexOffset = offsets.x;
	const uint vertexOffset = offsets.y;
	const Vertex v0 = UnpackVertex(vertexOffset + Indices[indexOffset + gl_PrimitiveID * 3 + 0]);
	const Vertex v1 = UnpackVertex(vertexOffset + Indices[indexOffset + gl_PrimitiveID * 3 + 1]);
	const Vertex v2 = UnpackVertex(vertexOffset + Indices[indexOffset + gl_PrimitiveID * 3 + 2]);
	const Material material = Materials[v0.materialIndex];

	// Compute the ray hit point properties.
    const vec3 barycentrics = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);
	const vec3 normal = normalize(Mix(v0.normal, v1.normal, v2.normal, barycentrics));
	const vec2 texCoord = Mix(v0.texCoord, v1.texCoord, v2.texCoord, barycentrics);
    
	const vec4 texColor = material.DiffuseTextureId >= 0 ? texture(TextureSamplers[material.DiffuseTextureId], texCoord) : vec4(1);
	const vec3 color = vec3(material.Diffuse.rgb * texColor.rgb);
    
    resultColor = color;
}
