#include "renderer/ray_tracing/bottom_level_acceleration_structure.h"
#include "renderer/buffers/buffer.h"
#include "renderer/buffers/storage_buffer.h"
#include "renderer/context/device.h"
#include "renderer/context/dynamic_functions.h"
#include "renderer/context/single_time_commands.h"

VkMemoryRequirements estun::BLAS::GetBufferMemoryRequirements(VkAccelerationStructureKHR accelerationStructure, VkAccelerationStructureMemoryRequirementsTypeKHR type)
{
    VkMemoryRequirements2 memoryRequirements2;
    memoryRequirements2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    memoryRequirements2.pNext = nullptr;

    VkAccelerationStructureMemoryRequirementsInfoKHR accelerationMemoryRequirements;
    accelerationMemoryRequirements.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
    accelerationMemoryRequirements.pNext = nullptr;
    accelerationMemoryRequirements.type = type;
    accelerationMemoryRequirements.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
    accelerationMemoryRequirements.accelerationStructure = accelerationStructure;
    FunctionsLocator::GetFunctions().vkGetAccelerationStructureMemoryRequirementsKHR(DeviceLocator::GetLogicalDevice(), &accelerationMemoryRequirements, &memoryRequirements2);

    return memoryRequirements2.memoryRequirements;
}

estun::BLAS::BLAS(
    std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer,
    uint32_t vertexCount, uint32_t indexCount,
    uint32_t vertexOffset, uint32_t indexOffset)
{
    VkAccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo = {};
    geometryTypeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
    geometryTypeInfo.pNext = nullptr;
    geometryTypeInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometryTypeInfo.maxPrimitiveCount = indexCount / 3;
    geometryTypeInfo.indexType = VK_INDEX_TYPE_UINT32;
    geometryTypeInfo.maxVertexCount = vertexCount;
    geometryTypeInfo.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometryTypeInfo.allowsTransforms = VK_FALSE;

    VkAccelerationStructureCreateInfoKHR structureCreateInfo = {};
    structureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    structureCreateInfo.pNext = nullptr;
    structureCreateInfo.compactedSize = 0;
    structureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    structureCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    structureCreateInfo.maxGeometryCount = 1;
    structureCreateInfo.pGeometryInfos = &geometryTypeInfo;
    structureCreateInfo.deviceAddress = VK_NULL_HANDLE;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkCreateAccelerationStructureKHR(DeviceLocator::GetLogicalDevice(), &structureCreateInfo, nullptr, &accelerationStructure_), "create acceleration structure");

    auto blasMemoryRequirements = GetBufferMemoryRequirements(accelerationStructure_, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR);
    buildScratchSize_ = GetBufferMemoryRequirements(accelerationStructure_, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR).size;
    objectSize_ = blasMemoryRequirements.size;

    std::shared_ptr<Buffer> scratchBuffer = std::make_shared<Buffer>(buildScratchSize_, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    std::shared_ptr<DeviceMemory> scratchMemory = std::make_shared<DeviceMemory>(scratchBuffer->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true));

    VkAccelerationStructureGeometryKHR geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.pNext = nullptr;
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry = {};
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    geometry.geometry.triangles.pNext = nullptr;
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry.geometry.triangles.vertexData.deviceAddress = vertexBuffer->GetDeviceAddress();
    geometry.geometry.triangles.vertexStride = sizeof(Vertex);
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometry.geometry.triangles.indexData.deviceAddress = indexBuffer->GetDeviceAddress();
    geometry.geometry.triangles.transformData.deviceAddress = 0;
    geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo = {};
    buildOffsetInfo.primitiveCount = indexCount / 3;
    buildOffsetInfo.primitiveOffset = indexOffset;
    buildOffsetInfo.firstVertex = vertexOffset;
    buildOffsetInfo.transformOffset = 0;

    accelerationGeometries_.push_back(geometry);
    buildOffsets_.push_back(&buildOffsetInfo);
    
    blasMemory_.reset(new DeviceMemory(objectSize_, blasMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true));

    VkBindAccelerationStructureMemoryInfoKHR bindMemoryInfo = {};
    bindMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
    bindMemoryInfo.pNext = nullptr;
    bindMemoryInfo.accelerationStructure = accelerationStructure_;
    bindMemoryInfo.memory = blasMemory_->GetMemory();
    bindMemoryInfo.memoryOffset = 0;
    bindMemoryInfo.deviceIndexCount = 0;
    bindMemoryInfo.pDeviceIndices = nullptr;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkBindAccelerationStructureMemoryKHR(DeviceLocator::GetLogicalDevice(), 1, &bindMemoryInfo), "bind acceleration structure");

    SingleTimeCommands::SubmitCompute([this, scratchBuffer](VkCommandBuffer commandBuffer) {
        const VkAccelerationStructureGeometryKHR *pGeometries = accelerationGeometries_.data();

        VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
        buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildGeometryInfo.pNext = nullptr;
        buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        buildGeometryInfo.update = false;
        buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
        buildGeometryInfo.dstAccelerationStructure = accelerationStructure_;
        buildGeometryInfo.geometryArrayOfPointers = VK_FALSE;
        buildGeometryInfo.geometryCount = 1;
        buildGeometryInfo.ppGeometries = &pGeometries;
        buildGeometryInfo.scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();

        FunctionsLocator::GetFunctions().vkCmdBuildAccelerationStructureKHR(commandBuffer, 1, &buildGeometryInfo, buildOffsets_.data());
    }, "create blas");
}


estun::BLAS::~BLAS()
{
    blasMemory_.reset();
    if (accelerationStructure_ != nullptr)
    {
        FunctionsLocator::GetFunctions().vkDestroyAccelerationStructureKHR(DeviceLocator::GetLogicalDevice(), accelerationStructure_, nullptr);
        accelerationStructure_ = nullptr;
    }
}

VkDeviceAddress estun::BLAS::GetDeviceAddress()
{
    VkAccelerationStructureDeviceAddressInfoKHR devAddrInfo;
    devAddrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    devAddrInfo.pNext = nullptr;
    devAddrInfo.accelerationStructure = accelerationStructure_;
    VkDeviceAddress deviceAddress = FunctionsLocator::GetFunctions().vkGetAccelerationStructureDeviceAddressKHR(DeviceLocator::GetLogicalDevice(), &devAddrInfo);

    return deviceAddress;
}

std::vector<std::shared_ptr<estun::BLAS>> estun::BLAS::CreateBlases(
    std::vector<std::shared_ptr<Model>> models,
    std::shared_ptr<VertexBuffer> vertexBuffer,
    std::shared_ptr<IndexBuffer> indexBuffer)
{
    ES_CORE_INFO("creating BLASes ...");
    std::vector<std::shared_ptr<BLAS>> blases;

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    for (auto &model : models)
    {
        const uint32_t vertexCount = static_cast<uint32_t>(model->SizeOfVertices());
        const uint32_t indexCount = static_cast<uint32_t>(model->SizeOfIndices());

        blases.push_back(std::make_shared<BLAS>(vertexBuffer, indexBuffer, vertexCount, indexCount, vertexOffset, indexOffset));

        vertexOffset += vertexCount;
        indexOffset += indexCount * sizeof(uint32_t);
    }

    ES_CORE_INFO("BLASes created");

    return blases;
}
