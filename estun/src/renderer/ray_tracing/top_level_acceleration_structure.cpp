#include "renderer/ray_tracing/top_level_acceleration_structure.h"
#include "renderer/ray_tracing/bottom_level_acceleration_structure.h"
#include "renderer/buffers/buffer.h"
#include "renderer/buffers/storage_buffer.h"
#include "renderer/context/device.h"
#include "renderer/context/dynamic_functions.h"
#include "renderer/context/single_time_commands.h"

uint32_t estun::TLAS::GetBufferSize(VkAccelerationStructureKHR accelerationStructure, VkAccelerationStructureMemoryRequirementsTypeKHR type)
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

    return memoryRequirements2.memoryRequirements.size;
}

estun::TLAS::TLAS(std::vector<std::shared_ptr<estun::BLAS>> blases)
{
    ES_CORE_INFO("creating TLAS ...");

    VkAccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo = {};
    geometryTypeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
    geometryTypeInfo.pNext = nullptr;
    geometryTypeInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometryTypeInfo.maxPrimitiveCount = blases.size();
    geometryTypeInfo.indexType = VK_INDEX_TYPE_NONE_KHR;
    geometryTypeInfo.maxVertexCount = 0;
    geometryTypeInfo.vertexFormat = VK_FORMAT_UNDEFINED;
    geometryTypeInfo.allowsTransforms = VK_FALSE;

    VkAccelerationStructureCreateInfoKHR structureCreateInfo = {};
    structureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    structureCreateInfo.pNext = nullptr;
    structureCreateInfo.compactedSize = 0;
    structureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    structureCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    structureCreateInfo.maxGeometryCount = 1;
    structureCreateInfo.pGeometryInfos = &geometryTypeInfo;
    structureCreateInfo.deviceAddress = 0;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkCreateAccelerationStructureKHR(DeviceLocator::GetLogicalDevice(), &structureCreateInfo, nullptr, &accelerationStructure_), "create acceleration structure");

    buildScratchSize_ = GetBufferSize(accelerationStructure_, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR);
    objectSize_ = GetBufferSize(accelerationStructure_, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR);

    std::shared_ptr<Buffer> scratchBuffer = std::make_shared<Buffer>(buildScratchSize_, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    std::shared_ptr<DeviceMemory> scratchMemory = std::make_shared<DeviceMemory>(scratchBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true));

    VkAccelerationStructureDeviceAddressInfoKHR devAddrInfo = {};
    devAddrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    devAddrInfo.pNext = nullptr;
    devAddrInfo.accelerationStructure = accelerationStructure_;
    deviceAddress_ = FunctionsLocator::GetFunctions().vkGetAccelerationStructureDeviceAddressKHR(DeviceLocator::GetLogicalDevice(), &devAddrInfo);

    std::vector<VkAccelerationStructureInstanceKHR> geometryInstances;
    uint32_t instanceId = 0;
    for (auto &blas : blases)
    {
        VkAccelerationStructureInstanceKHR geometryInstance = {};
        std::memcpy(&geometryInstance.transform, blas->GetTransformMatrix(), sizeof(glm::mat4)); //sizeof() бы приделать
        geometryInstance.instanceCustomIndex = instanceId;
        geometryInstance.mask = 0xFF;
        geometryInstance.instanceShaderBindingTableRecordOffset = blas->GetHitGroup();
        geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; // Disable culling - more fine control could be provided by the application
        geometryInstance.accelerationStructureReference = blas->GetDeviceAddress();
        geometryInstances.push_back(geometryInstance);
        instanceId++;
    }

    uint32_t instancesSize = geometryInstances.size() * sizeof(VkAccelerationStructureInstanceKHR);
    std::shared_ptr<Buffer> instancesBuffer = std::make_shared<Buffer>(instancesSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    std::shared_ptr<DeviceMemory> instancesMemory = std::make_shared<DeviceMemory>(instancesBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true));

    const auto data = instancesMemory->Map(0, instancesSize);
    std::memcpy(data, geometryInstances.data(), instancesSize);
    instancesMemory->Unmap();

    VkAccelerationStructureGeometryKHR geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.pNext = nullptr;
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    geometry.geometry = {};
    geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometry.geometry.instances.pNext = nullptr;
    geometry.geometry.instances.arrayOfPointers = VK_FALSE;
    geometry.geometry.instances.data.deviceAddress = instancesBuffer->GetDeviceAddress();

    VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo = {};
    buildOffsetInfo.primitiveCount = blases.size();
    buildOffsetInfo.primitiveOffset = 0;
    buildOffsetInfo.firstVertex = 0;
    buildOffsetInfo.transformOffset = 0;

    accelerationGeometries_.push_back(geometry);
    buildOffsets_.push_back(&buildOffsetInfo);

    std::shared_ptr<Buffer> tlasesBuffer = std::make_shared<Buffer>(objectSize_, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    std::shared_ptr<DeviceMemory> tlasesMemory = std::make_shared<DeviceMemory>(tlasesBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true));

    VkBindAccelerationStructureMemoryInfoKHR bindMemoryInfo = {};
    bindMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
    bindMemoryInfo.pNext = nullptr;
    bindMemoryInfo.accelerationStructure = accelerationStructure_;
    bindMemoryInfo.memory = tlasesMemory->GetMemory();
    bindMemoryInfo.memoryOffset = 0;
    bindMemoryInfo.deviceIndexCount = 0;
    bindMemoryInfo.pDeviceIndices = nullptr;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkBindAccelerationStructureMemoryKHR(DeviceLocator::GetLogicalDevice(), 1, &bindMemoryInfo), "bind acceleration structure");

    SingleTimeCommands::SubmitCompute(CommandPoolLocator::GetComputePool(), [this, scratchBuffer](VkCommandBuffer commandBuffer) {
        const VkAccelerationStructureGeometryKHR *pGeometries = accelerationGeometries_.data();

        VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
        buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildGeometryInfo.pNext = nullptr;
        buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        buildGeometryInfo.update = VK_FALSE;
        buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
        buildGeometryInfo.dstAccelerationStructure = accelerationStructure_;
        buildGeometryInfo.geometryArrayOfPointers = VK_FALSE;
        buildGeometryInfo.geometryCount = 1;
        buildGeometryInfo.ppGeometries = &pGeometries;
        buildGeometryInfo.scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();

        FunctionsLocator::GetFunctions().vkCmdBuildAccelerationStructureKHR(commandBuffer, 1, &buildGeometryInfo, buildOffsets_.data());
    });

    ES_CORE_INFO("TLAS created");
}

estun::TLAS::~TLAS()
{
    if (accelerationStructure_ != nullptr)
    {
        FunctionsLocator::GetFunctions().vkDestroyAccelerationStructureKHR(DeviceLocator::GetLogicalDevice(), accelerationStructure_, nullptr);
        accelerationStructure_ = nullptr;
    }
}

estun::DescriptableInfo estun::TLAS::GetInfo()
{
    VkWriteDescriptorSetAccelerationStructureKHR structureInfo = {};
    structureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    structureInfo.pNext = nullptr;
    structureInfo.accelerationStructureCount = 1;
    structureInfo.pAccelerationStructures = &accelerationStructure_;

    DescriptableInfo info;
    info.asI = structureInfo;

    return info;
}