#pragma once

#include "renderer/material/vulkan_descriptor_pool.h"
#include "renderer/material/vulkan_descriptor_sets.h"
#include "renderer/material/vulkan_texture_image.h"
#include "renderer/material/vulkan_texture_sampler.h"
#include "renderer/material/vulkan_texture_image_view.h"
#include "renderer/material/vulkan_pipeline.h"

#include "renderer/buffers/vulkan_uniform_buffer.h"
#include "renderer/buffers/vulkan_vertex_buffer.h"

namespace estun
{
    class VulkanMaterial
    {
    private:
		std::shared_ptr<VulkanGraphicsPipeline> pipeline = nullptr;
		std::shared_ptr<VulkanComputePipeline>  computePipeline = nullptr;
        std::shared_ptr<VulkanTextureImage>     textureImage = nullptr;
        std::shared_ptr<VulkanTextureImageView> textureImageView = nullptr;
        std::shared_ptr<VulkanTextureSampler>   textureSampler = nullptr;

        VulkanDescriptorSets*  descriptorSets  = nullptr;
        VulkanDescriptorPool*  descriptorPool = nullptr;
        VulkanDescriptorSets*  computeDescriptorSets  = nullptr;
        VulkanDescriptorPool*  computeDescriptorPool = nullptr;

        bool hasCompute = false;

    public:
        VulkanMaterial(bool compute = false);
        ~VulkanMaterial();

        void CreatePipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
        void CreateComputePipeline(const std::string& computeShaderPath);
        void CreateTexture(const std::string& texturePath);
        void CreateDescriptorSets(VulkanUniformBuffer* uniformBuffer);
        void CreateComputeDescriptorSets(
            VulkanVertexBuffer* vboA,
            VulkanVertexBuffer* vboB,
            VulkanVertexBuffer* vboC
        );

        void Update(
            VulkanVertexBuffer* vboA,
            VulkanVertexBuffer* vboB,
            VulkanVertexBuffer* vboC
        );

        void DeletePipeline();

        std::shared_ptr<VulkanGraphicsPipeline> GetPipeline() const;
        std::shared_ptr<VulkanComputePipeline>  GetComputePipeline();
        VulkanDescriptorSets*                   GetDescriptorSets();
        VulkanDescriptorSets*                   GetComputeDescriptorSets();
    };
}
