#include "estun.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>
#include "tiny_obj_loader.h"

#include "cornell_box.h"
#include "renderer/context/single_time_commands.h"

#define WIDTH 600
#define HEIGHT 400

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

Camera camera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, -1.0f, 0.0f));

void processInput(int key, int scancode, int action, int mods);
void mouse_callback(double xpos, double ypos);
void scroll_callback(double xoffset, double yoffset);
void mouse_button_callback(int button, int action, int mods);
void framebuffer_size_callback(int width, int height);

estun::WindowConfig winConf = {"Ray tracing", WIDTH, HEIGHT, "assets/textures/icon.png", false, false, true};
estun::GameInfo info("test", {0, 0, 1}, WIDTH, HEIGHT, true, false, true);
std::unique_ptr<estun::Window> window;

struct VertexUBO
{
    glm::mat4 modelView;
    glm::mat4 projection;
    glm::mat4 modelViewInverse;
    glm::mat4 projectionInverse;
    float aperture;
    float focusDistance;
    uint32_t totalNumberOfSamples;
    uint32_t numberOfSamples;
    uint32_t numberOfBounces;
    uint32_t randomSeed;
    uint32_t gammaCorrection;
    uint32_t hasSky;
};

struct AccelerationMemory
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    uint64_t memoryAddress = 0;
    void *mappedPointer = nullptr;
};

typedef struct AccelerationMemory MappedBuffer;

#define ASSERT_VK_RESULT(r)                                                                    \
    {                                                                                          \
        VkResult result = (r);                                                                 \
        if (result != VK_SUCCESS) {                                                            \
            std::cout << "Vulkan Assertion failed in Line " << __LINE__ << " with: " << result \
                      << std::endl;                                                            \
        }                                                                                      \
    }
    
VkAccelerationStructureKHR topLevelAS = VK_NULL_HANDLE;
uint64_t topLevelASHandle = 0;

MappedBuffer shaderBindingTable = {};
uint32_t shaderBindingTableSize = 0;
uint32_t shaderBindingTableGroupCount = 3;

int surface_size = 256;
float surface_scale = 10.0f;
bool wireframe = false;


VkMemoryRequirements GetAccelerationStructureMemoryRequirements(
    VkAccelerationStructureKHR acceleration,
    VkAccelerationStructureMemoryRequirementsTypeKHR type) {
    VkMemoryRequirements2 memoryRequirements2;
    memoryRequirements2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    memoryRequirements2.pNext = nullptr;

    VkAccelerationStructureMemoryRequirementsInfoKHR accelerationMemoryRequirements;
    accelerationMemoryRequirements.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
    accelerationMemoryRequirements.pNext = nullptr;
    accelerationMemoryRequirements.type = type;
    accelerationMemoryRequirements.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
    accelerationMemoryRequirements.accelerationStructure = acceleration;
    estun::FunctionsLocator::GetFunctions().vkGetAccelerationStructureMemoryRequirementsKHR(estun::DeviceLocator::GetLogicalDevice(), &accelerationMemoryRequirements,
                                                    &memoryRequirements2);

    return memoryRequirements2.memoryRequirements;
}

void BindAccelerationMemory(VkAccelerationStructureKHR acceleration, VkDeviceMemory memory) {

    VkBindAccelerationStructureMemoryInfoKHR accelerationMemoryBindInfo;
    accelerationMemoryBindInfo.sType =
        VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
    accelerationMemoryBindInfo.pNext = nullptr;
    accelerationMemoryBindInfo.accelerationStructure = acceleration;
    accelerationMemoryBindInfo.memory = memory;
    accelerationMemoryBindInfo.memoryOffset = 0;
    accelerationMemoryBindInfo.deviceIndexCount = 0;
    accelerationMemoryBindInfo.pDeviceIndices = nullptr;

    ASSERT_VK_RESULT(estun::FunctionsLocator::GetFunctions().vkBindAccelerationStructureMemoryKHR(estun::DeviceLocator::GetLogicalDevice(), 1, &accelerationMemoryBindInfo));
}

uint64_t GetBufferAddress(VkBuffer buffer) {

    VkBufferDeviceAddressInfoKHR bufferAddressInfo;
    bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferAddressInfo.pNext = nullptr;
    bufferAddressInfo.buffer = buffer;

    return vkGetBufferDeviceAddress(estun::DeviceLocator::GetLogicalDevice(), &bufferAddressInfo);
}

MappedBuffer CreateMappedBuffer(void* srcData, uint32_t byteLength) {
    MappedBuffer out = {};

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = byteLength;
    bufferInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;
    ASSERT_VK_RESULT(vkCreateBuffer(estun::DeviceLocator::GetLogicalDevice(), &bufferInfo, nullptr, &out.buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(estun::DeviceLocator::GetLogicalDevice(), out.buffer, &memoryRequirements);

    VkMemoryAllocateFlagsInfo memAllocFlagsInfo;
    memAllocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memAllocFlagsInfo.pNext = nullptr;
    memAllocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    memAllocFlagsInfo.deviceMask = 0;

    VkMemoryAllocateInfo memAllocInfo;
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = &memAllocFlagsInfo;
    memAllocInfo.allocationSize = memoryRequirements.size;
    memAllocInfo.memoryTypeIndex =
        estun::DeviceMemory::FindMemoryType(memoryRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ASSERT_VK_RESULT(vkAllocateMemory(estun::DeviceLocator::GetLogicalDevice(), &memAllocInfo, nullptr, &out.memory));

    ASSERT_VK_RESULT(vkBindBufferMemory(estun::DeviceLocator::GetLogicalDevice(), out.buffer, out.memory, 0));

    out.memoryAddress = GetBufferAddress(out.buffer);

    void* dstData;
    ASSERT_VK_RESULT(vkMapMemory(estun::DeviceLocator::GetLogicalDevice(), out.memory, 0, byteLength, 0, &dstData));
    if (srcData != nullptr) {
        memcpy(dstData, srcData, byteLength);
    }
    vkUnmapMemory(estun::DeviceLocator::GetLogicalDevice(), out.memory);
    out.mappedPointer = dstData;

    return out;
}

AccelerationMemory CreateAccelerationScratchBuffer(
    VkAccelerationStructureKHR acceleration,
    VkAccelerationStructureMemoryRequirementsTypeKHR type) {
    AccelerationMemory out = {};

    VkMemoryRequirements asRequirements =
        GetAccelerationStructureMemoryRequirements(acceleration, type);

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = asRequirements.size;
    bufferInfo.usage =
        VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;
    ASSERT_VK_RESULT(vkCreateBuffer(estun::DeviceLocator::GetLogicalDevice(), &bufferInfo, nullptr, &out.buffer));

    VkMemoryRequirements bufRequirements;
    vkGetBufferMemoryRequirements(estun::DeviceLocator::GetLogicalDevice(), out.buffer, &bufRequirements);

    // buffer requirements can differ to AS requirements, so we max them
    uint64_t alloctionSize =
        asRequirements.size > bufRequirements.size ? asRequirements.size : bufRequirements.size;
    // combine AS and buf mem types
    uint32_t allocationMemoryBits = bufRequirements.memoryTypeBits | asRequirements.memoryTypeBits;

    VkMemoryAllocateFlagsInfo memAllocFlagsInfo;
    memAllocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memAllocFlagsInfo.pNext = nullptr;
    memAllocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    memAllocFlagsInfo.deviceMask = 0;

    VkMemoryAllocateInfo memAllocInfo = {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = &memAllocFlagsInfo;
    memAllocInfo.allocationSize = alloctionSize;
    memAllocInfo.memoryTypeIndex =
        estun::DeviceMemory::FindMemoryType(allocationMemoryBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ASSERT_VK_RESULT(vkAllocateMemory(estun::DeviceLocator::GetLogicalDevice(), &memAllocInfo, nullptr, &out.memory));

    ASSERT_VK_RESULT(vkBindBufferMemory(estun::DeviceLocator::GetLogicalDevice(), out.buffer, out.memory, 0));

    out.memoryAddress = GetBufferAddress(out.buffer);

    return out;
}

VkAccelerationStructureKHR bottomLevelAS = VK_NULL_HANDLE;
uint64_t bottomLevelASHandle = 0;

uint32_t GetBufferSize(VkAccelerationStructureKHR accelerationStructure, VkAccelerationStructureMemoryRequirementsTypeKHR type)
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
    estun::FunctionsLocator::GetFunctions().vkGetAccelerationStructureMemoryRequirementsKHR(estun::DeviceLocator::GetLogicalDevice(), &accelerationMemoryRequirements, &memoryRequirements2);

    return memoryRequirements2.memoryRequirements.size;
}

int main(int argc, const char **argv)
{
    estun::Log::Init();
    window = std::make_unique<estun::Window>(winConf);
    window->OnKey = processInput;
    window->OnCursorPosition = mouse_callback;
    window->OnScroll = scroll_callback;
    window->OnMouseButton = mouse_button_callback;
    window->OnResize = framebuffer_size_callback;
    window->ToggleCursor(false);

    std::shared_ptr<estun::Context> context = std::make_shared<estun::Context>(window->GetWindow(), &info);
    estun::ContextLocator::Provide(context.get());

    VertexUBO ubo = {};
    ubo.aperture = 0.5f;
    ubo.focusDistance = 1.0f;
    ubo.totalNumberOfSamples = 0;
    ubo.numberOfSamples = 8;
    ubo.numberOfBounces = 2;
    ubo.randomSeed = 1;
    ubo.gammaCorrection = false;
    ubo.hasSky = false;

    std::vector<estun::UniformBuffer<VertexUBO>> UBs(context->GetSwapChain()->GetImageViews().size());
    std::vector<std::shared_ptr<estun::Model>> models;

    models.push_back(std::make_shared<estun::Model>(CornellBox::CreateCornellBox(1.0f)));

    std::vector<estun::Texture> textures;

    std::vector<estun::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<estun::Material> materials;

    for (const auto &model : models)
    {
        const auto vertexOffset = static_cast<uint32_t>(vertices.size());
        const auto materialOffset = static_cast<uint32_t>(materials.size());

        vertices.insert(vertices.end(), model->GetVertices().begin(), model->GetVertices().end());
        indices.insert(indices.end(), model->GetIndices().begin(), model->GetIndices().end());
        materials.insert(materials.end(), model->GetMaterials().begin(), model->GetMaterials().end());

        for (size_t i = vertexOffset; i != vertices.size(); ++i)
        {
            vertices[i].materialIndex += materialOffset;
        }
    }

    std::shared_ptr<estun::VertexBuffer> VB = std::make_shared<estun::VertexBuffer>(vertices);
    std::shared_ptr<estun::IndexBuffer> IB = std::make_shared<estun::IndexBuffer>(indices);
    std::shared_ptr<estun::StorageBuffer<estun::Material>> materialBuffer = std::make_shared<estun::StorageBuffer<estun::Material>>(materials);

    std::vector<std::shared_ptr<estun::BLAS>> blases = estun::BLAS::CreateBlases(models, VB, IB);
    std::shared_ptr<estun::TLAS> tlas = std::make_shared<estun::TLAS>(blases);

    auto extent = estun::ContextLocator::GetSwapChain()->GetExtent();
    std::shared_ptr<estun::Image> storeImage = estun::Image::CreateStorageImage(extent.width, extent.height);
    storeImage->ToLayout(VK_IMAGE_LAYOUT_GENERAL);

    std::vector<estun::DescriptorBinding> descriptorBindings = {
        estun::DescriptorBinding::AccelerationStructure(0, tlas, VK_SHADER_STAGE_RAYGEN_BIT_KHR),
        estun::DescriptorBinding::StorageImage(1, storeImage, VK_SHADER_STAGE_RAYGEN_BIT_KHR)};

    std::shared_ptr<estun::Descriptor> descriptor = std::make_shared<estun::Descriptor>(descriptorBindings, context->GetSwapChain()->GetImageViews().size());
    descriptorBindings.clear();

    std::shared_ptr<estun::RayTracingRender> render = context->CreateRayTracingRender();
    
    std::shared_ptr<estun::RayTracingPipeline> pipeline = render->CreatePipeline(
        {{"assets/shaders/ray-generation.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR},
         {"assets/shaders/ray-miss.spv", VK_SHADER_STAGE_MISS_BIT_KHR},
         {"assets/shaders/ray-closest-hit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR}},
        descriptor);
    std::vector<uint32_t> v = {0, 1, 2};
    
    std::shared_ptr<estun::ShaderBindingTable> shaderBindingTable = std::make_shared<estun::ShaderBindingTable>(pipeline, v);

    context->WriteBuffers([&]() {
        render->BeginBuffer();
        render->Bind(pipeline);
        render->Bind(descriptor);
        render->TraceRays(shaderBindingTable, storeImage->GetImage().GetWidth(), storeImage->GetImage().GetHeight());
        context->CopyImageToSwapChain(render->GetCurrCommandBuffer(), storeImage);
        render->EndBuffer();
    });

    while (!glfwWindowShouldClose(window->GetWindow()))
    {
        float currFrame = window->Time();
        deltaTime = lastFrame - currFrame;
        lastFrame = currFrame;
        glfwPollEvents();

        context->StartDraw();

        UBs[context->GetImageIndex()].SetValue(ubo);

        context->SubmitDraw();
    }

    pipeline.reset();
    render.reset();
    context->Clear();
    blases.clear();
    tlas.reset();
    shaderBindingTable.reset();
    storeImage.reset();
    UBs.clear();
    VB.reset();
    IB.reset();
    textures.clear();
    materialBuffer.reset();
    descriptor.reset();
    window.reset();
    context.reset();
    return 0;
}

bool cursor = false;

void processInput(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        window->Close();
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT) && !cursor)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT) && !cursor)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT) && !cursor)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT) && !cursor)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        wireframe = !wireframe;
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        window->ToggleCursor(!cursor);
        cursor = !cursor;
        ES_CORE_INFO(cursor);
    }
}

void mouse_callback(double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(int button, int action, int mods)
{
}

void framebuffer_size_callback(int width, int height)
{
    info.width_ = width;
    info.height_ = height;
}
