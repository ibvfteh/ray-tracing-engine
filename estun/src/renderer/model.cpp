#include "renderer/model.h"
#include "core/core.h"
#include "renderer/context.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

estun::Model estun::Model::LoadModel(const std::string &name, const std::string &filename)
{
    ES_CORE_INFO(std::string("Loading '") + filename + std::string("'... "));

    const std::string materialPath = std::filesystem::path(filename).parent_path().string();

    tinyobj::ObjReader objReader;

    if (!objReader.ParseFromFile(filename))
    {
        ES_CORE_ASSERT(std::string("Failed to load model '") + filename + std::string("':\n") + objReader.Error());
    }

    if (!objReader.Warning().empty())
    {
        ES_CORE_WARN(objReader.Warning());
    }

    // Materials
    std::vector<Material> materials;

    for (const auto &material : objReader.GetMaterials())
    {
        Material m{};

        m.diffuse_ = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0);
        m.diffuseTextureId_ = -1;

        materials.emplace_back(m);
    }

    if (materials.empty())
    {
        Material m{};

        m.diffuse_ = glm::vec4(0.7f, 0.7f, 0.7f, 1.0);
        m.diffuseTextureId_ = -1;

        materials.emplace_back(m);
    }

    // Geometry
    const auto &objAttrib = objReader.GetAttrib();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices(objAttrib.vertices.size());
    size_t faceId = 0;

    for (const auto &shape : objReader.GetShapes())
    {
        const auto &mesh = shape.mesh;

        for (const auto &index : mesh.indices)
        {
            Vertex vertex = {};
            vertex.position =
                {
                    objAttrib.vertices[3 * index.vertex_index + 0],
                    objAttrib.vertices[3 * index.vertex_index + 1],
                    objAttrib.vertices[3 * index.vertex_index + 2],
                };

            vertex.normal =
                {
                    objAttrib.normals[3 * index.normal_index + 0],
                    objAttrib.normals[3 * index.normal_index + 1],
                    objAttrib.normals[3 * index.normal_index + 2]};

            if (!objAttrib.texcoords.empty())
            {
                vertex.texCoord =
                    {
                        objAttrib.texcoords[2 * index.texcoord_index + 0],
                        1 - objAttrib.texcoords[2 * index.texcoord_index + 1]};
            }

            //vertex.materialIndex = std::max(0, mesh.material_ids[faceId++ / 3]);

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    ES_CORE_INFO(std::string("(") +
                 std::to_string(vertices.size()) + std::string(" vertices, ") +
                 std::to_string(uniqueVertices.size()) + std::string(" unique vertices, ") +
                 std::to_string(materials.size()) + std::string(" materials)"));

    return Model(name, std::move(vertices), std::move(indices), std::move(materials));
}

estun::Model estun::Model::CreateBox(const glm::vec3 &p0, const glm::vec3 &p1, const Material &material)
{
    std::vector<Vertex> vertices =
        {
            Vertex{glm::vec3(p0.x, p0.y, p0.z), glm::vec3(-1, 0, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p0.y, p1.z), glm::vec3(-1, 0, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p1.y, p1.z), glm::vec3(-1, 0, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p1.y, p0.z), glm::vec3(-1, 0, 0), glm::vec2(0), 0},

            Vertex{glm::vec3(p1.x, p0.y, p1.z), glm::vec3(1, 0, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p0.y, p0.z), glm::vec3(1, 0, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p1.y, p0.z), glm::vec3(1, 0, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p1.y, p1.z), glm::vec3(1, 0, 0), glm::vec2(0), 0},

            Vertex{glm::vec3(p1.x, p0.y, p0.z), glm::vec3(0, 0, -1), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p0.y, p0.z), glm::vec3(0, 0, -1), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p1.y, p0.z), glm::vec3(0, 0, -1), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p1.y, p0.z), glm::vec3(0, 0, -1), glm::vec2(0), 0},

            Vertex{glm::vec3(p0.x, p0.y, p1.z), glm::vec3(0, 0, 1), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p0.y, p1.z), glm::vec3(0, 0, 1), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p1.y, p1.z), glm::vec3(0, 0, 1), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p1.y, p1.z), glm::vec3(0, 0, 1), glm::vec2(0), 0},

            Vertex{glm::vec3(p0.x, p0.y, p0.z), glm::vec3(0, -1, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p0.y, p0.z), glm::vec3(0, -1, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p0.y, p1.z), glm::vec3(0, -1, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p0.y, p1.z), glm::vec3(0, -1, 0), glm::vec2(0), 0},

            Vertex{glm::vec3(p1.x, p1.y, p0.z), glm::vec3(0, 1, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p1.y, p0.z), glm::vec3(0, 1, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p0.x, p1.y, p1.z), glm::vec3(0, 1, 0), glm::vec2(0), 0},
            Vertex{glm::vec3(p1.x, p1.y, p1.z), glm::vec3(0, 1, 0), glm::vec2(0), 0},
        };

    std::vector<uint32_t> indices =
        {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23};

    return Model(
        "cube",
        std::move(vertices),
        std::move(indices),
        std::vector<Material>{material});
}

estun::Model estun::Model::CreateSphere(const glm::vec3 &center, float radius, const Material &material)
{
    const int slices = 32;
    const int stacks = 16;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    const float pi = 3.14159265358979f;

    for (int j = 0; j <= stacks; ++j)
    {
        const float j0 = pi * j / stacks;

        // Vertex
        const float v = radius * -std::sin(j0);
        const float z = radius * std::cos(j0);

        // Normals
        const float n0 = -std::sin(j0);
        const float n1 = std::cos(j0);

        for (int i = 0; i <= slices; ++i)
        {
            const float i0 = 2 * pi * i / slices;

            const glm::vec3 position(
                center.x + v * std::sin(i0),
                center.y + z,
                center.z + v * std::cos(i0));

            const glm::vec3 normal(
                n0 * std::sin(i0),
                n1,
                n0 * std::cos(i0));

            const glm::vec2 texCoord(
                static_cast<float>(i) / slices,
                static_cast<float>(j) / stacks);

            vertices.push_back(Vertex{position, normal, texCoord, 0});
        }
    }

    for (int j = 0; j < stacks; ++j)
    {
        for (int i = 0; i < slices; ++i)
        {
            const auto j0 = (j + 0) * (slices + 1);
            const auto j1 = (j + 1) * (slices + 1);
            const auto i0 = i + 0;
            const auto i1 = i + 1;

            indices.push_back(j0 + i0);
            indices.push_back(j1 + i0);
            indices.push_back(j1 + i1);

            indices.push_back(j0 + i0);
            indices.push_back(j1 + i1);
            indices.push_back(j0 + i1);
        }
    }

    return Model(
        "sphere",
        std::move(vertices),
        std::move(indices),
        std::vector<Material>{material});
}

void estun::Model::SetMaterial(const Material &material)
{
    if (materials_.size() != 1)
    {
        ES_CORE_ASSERT("Cannot change material on a multi-material model");
    }

    materials_[0] = material;
}

void estun::Model::Transform(const glm::mat4 &transform)
{
    const auto transformIT = glm::inverseTranspose(transform);

    for (auto &vertex : vertices_)
    {
        vertex.position = transform * glm::vec4(vertex.position, 1);
        vertex.normal = transformIT * glm::vec4(vertex.normal, 0);
    }
}

estun::Model::Model(const std::string &name, std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices, std::vector<Material> &&materials)
    : name_(name),
      vertices_(std::move(vertices)),
      indices_(std::move(indices)),
      materials_(std::move(materials)),
      verticesSize_(vertices_.size()),
      indicesSize_(indices_.size()),
      materialsSize_(materials.size())
{
}
