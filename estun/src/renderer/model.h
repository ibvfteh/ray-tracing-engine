#pragma once

#include "renderer/common.h"
#include "includes/glm.h"
#include "renderer/buffers/vertex.h"
#include "renderer/material/material.h"

namespace estun
{

class Model
{
public:
    static Model LoadModel(const std::string &name, const std::string &filename);
    static Model CreateBox(const glm::vec3 &p0, const glm::vec3 &p1, const Material &material);
    static Model CreateSphere(const glm::vec3 &center, float radius, const Material &material);

    Model &operator=(const Model &) = delete;
    Model &operator=(Model &&) = delete;

    Model() = default;
    Model(const Model &) = default;
    Model(Model &&) = default;
    ~Model() = default;

    Model(const std::string &name, std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices, std::vector<Material> &&materials);

    void SetMaterial(const Material &material);
    void Transform(const glm::mat4 &transform);

    const std::vector<Vertex> &GetVertices() const { return vertices_; }
    const std::vector<uint32_t> &GetIndices() const { return indices_; }
    const std::vector<Material> &GetMaterials() const { return materials_; }
    const std::string &GetName() const { return name_; }

    uint32_t SizeOfVertices() const { return verticesSize_; }   //static_cast<uint32_t>(vertices_.size())
    uint32_t SizeOfIndices() const { return indicesSize_; }     //static_cast<uint32_t>(indices_.size())
    uint32_t SizeOfMaterials() const { return materialsSize_; } //static_cast<uint32_t>(materials_.size())

private:
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
    std::vector<Material> materials_;
    std::string name_;

    uint32_t verticesSize_;
    uint32_t indicesSize_;
    uint32_t materialsSize_;
};

} // namespace estun