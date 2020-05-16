#include "cornell_box.h"

namespace
{
	void AddTriangle(std::vector<uint32_t>& indices, const uint32_t offset, const uint32_t i0, const uint32_t i1, const uint32_t i2)
	{
		indices.push_back(offset + i0);
		indices.push_back(offset + i1);
		indices.push_back(offset + i2);
	}
}

estun::Model CornellBox::CreateCornellBox(const float scale)
{
	std::vector<estun::Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<estun::Material> materials;

	CornellBox::Create(scale, vertices, indices, materials);

	return estun::Model(
		"box",
		std::move(vertices),
		std::move(indices),
		std::move(materials)
	);
}

void CornellBox::Create(
	const float scale,
	std::vector<estun::Vertex>& vertices,
	std::vector<uint32_t>& indices,
	std::vector<estun::Material>& materials)
{
	materials.push_back(estun::Material::Lambertian(glm::vec3(0.65f, 0.05f, 0.05f))); // red
	materials.push_back(estun::Material::Lambertian(glm::vec3(0.12f, 0.45f, 0.15f))); // green
	materials.push_back(estun::Material::Lambertian(glm::vec3(0.73f, 0.73f, 0.73f))); // white
	materials.push_back(estun::Material::DiffuseLight(glm::vec3(15.0f))); // light

	const float s = scale;

	const glm::vec3 l0(-s, -s, 0);
	const glm::vec3 l1(-s, -s, -s);
	const glm::vec3 l2(-s, s, -s);
	const glm::vec3 l3(-s, s, 0);

	const glm::vec3 r0(s, -s, 0);
	const glm::vec3 r1(s, -s, -s);
	const glm::vec3 r2(s, s, -s);
	const glm::vec3 r3(s, s, 0);

	// Left green panel
	auto i = static_cast<uint32_t>(vertices.size());
	vertices.push_back(estun::Vertex{ l0, glm::vec3(1, 0, 0), glm::vec2(0, 1), 1 });
	vertices.push_back(estun::Vertex{ l1, glm::vec3(1, 0, 0), glm::vec2(1, 1), 1 });
	vertices.push_back(estun::Vertex{ l2, glm::vec3(1, 0, 0), glm::vec2(1, 0), 1 });
	vertices.push_back(estun::Vertex{ l3, glm::vec3(1, 0, 0), glm::vec2(0, 0), 1 });

	AddTriangle(indices, i, 0, 1, 2);
	AddTriangle(indices, i, 0, 2, 3);

	// Right red panel
	i = static_cast<uint32_t>(vertices.size());
	vertices.push_back(estun::Vertex{ r0, glm::vec3(-1, 0, 0), glm::vec2(0, 1), 0 });
	vertices.push_back(estun::Vertex{ r1, glm::vec3(-1, 0, 0), glm::vec2(1, 1), 0 });
	vertices.push_back(estun::Vertex{ r2, glm::vec3(-1, 0, 0), glm::vec2(1, 0), 0 });
	vertices.push_back(estun::Vertex{ r3, glm::vec3(-1, 0, 0), glm::vec2(0, 0), 0 });

	AddTriangle(indices, i, 2, 1, 0);
	AddTriangle(indices, i, 3, 2, 0);

	// Back white panel
	i = static_cast<uint32_t>(vertices.size());
	vertices.push_back(estun::Vertex{ l1, glm::vec3(0, 0, 1), glm::vec2(0, 1), 2 });
	vertices.push_back(estun::Vertex{ r1, glm::vec3(0, 0, 1), glm::vec2(1, 1), 2 });
	vertices.push_back(estun::Vertex{ r2, glm::vec3(0, 0, 1), glm::vec2(1, 0), 2 });
	vertices.push_back(estun::Vertex{ l2, glm::vec3(0, 0, 1), glm::vec2(0, 0), 2 });

	AddTriangle(indices, i, 0, 1, 2);
	AddTriangle(indices, i, 0, 2, 3);

	// Bottom white panel
	i = static_cast<uint32_t>(vertices.size());
	vertices.push_back(estun::Vertex{ l0, glm::vec3(0, 1, 0), glm::vec2(0, 1), 2 });
	vertices.push_back(estun::Vertex{ r0, glm::vec3(0, 1, 0), glm::vec2(1, 1), 2 });
	vertices.push_back(estun::Vertex{ r1, glm::vec3(0, 1, 0), glm::vec2(1, 0), 2 });
	vertices.push_back(estun::Vertex{ l1, glm::vec3(0, 1, 0), glm::vec2(0, 0), 2 });

	AddTriangle(indices, i, 0, 1, 2);
	AddTriangle(indices, i, 0, 2, 3);

	// Top white panel
	i = static_cast<uint32_t>(vertices.size());
	vertices.push_back(estun::Vertex{ l2, glm::vec3(0, -1, 0), glm::vec2(0, 1), 2 });
	vertices.push_back(estun::Vertex{ r2, glm::vec3(0, -1, 0), glm::vec2(1, 1), 2 });
	vertices.push_back(estun::Vertex{ r3, glm::vec3(0, -1, 0), glm::vec2(1, 0), 2 });
	vertices.push_back(estun::Vertex{ l3, glm::vec3(0, -1, 0), glm::vec2(0, 0), 2 });

	AddTriangle(indices, i, 0, 1, 2);
	AddTriangle(indices, i, 0, 2, 3);

	// Light
	i = static_cast<uint32_t>(vertices.size());

	const float x0 = s * (213.0f / 555.0f);
	const float x1 = s * (343.0f / 555.0f);
	const float z0 = s * (-555.0f + 332.0f) / 555.0f;
	const float z1 = s * (-555.0f + 227.0f) / 555.0f;
	const float y1 = s * 0.998f;

	vertices.push_back(estun::Vertex{ glm::vec3(x0, y1, z1), glm::vec3(0, -1, 0), glm::vec2(0, 1), 3 });
	vertices.push_back(estun::Vertex{ glm::vec3(x1, y1, z1), glm::vec3(0, -1, 0), glm::vec2(1, 1), 3 });
	vertices.push_back(estun::Vertex{ glm::vec3(x1, y1, z0), glm::vec3(0, -1, 0), glm::vec2(1, 0), 3 });
	vertices.push_back(estun::Vertex{ glm::vec3(x0, y1, z0), glm::vec3(0, -1, 0), glm::vec2(0, 0), 3 });

	AddTriangle(indices, i, 0, 1, 2);
	AddTriangle(indices, i, 0, 2, 3);
}