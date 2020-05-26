#include "estun.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>
#include "tiny_obj_loader.h"

#include "cornell_box.h"

#define WIDTH 1200
#define HEIGHT 800

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

void processInput(int key, int scancode, int action, int mods);
void mouse_callback(double xpos, double ypos);
void scroll_callback(double xoffset, double yoffset);
void mouse_button_callback(int button, int action, int mods);
void framebuffer_size_callback(int width, int height);

estun::WindowConfig winConf = {"Ray tracing", WIDTH, HEIGHT, "assets/textures/icon.png", false, false, true};
estun::GameInfo info("test", {0, 0, 1}, WIDTH, HEIGHT, true, false, true);
std::unique_ptr<estun::Window> window;

struct CameraUBO
{
    glm::vec4 camPos;
    glm::vec4 camDir;
    glm::vec4 camUp;
    glm::vec4 camSide;
    glm::vec4 camNearFarFov;
    uint32_t totalNumberOfSamples;
    uint32_t numberOfSamples;
    uint32_t numberOfBounces;
};
/*
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
*/

int surface_size = 256;
float surface_scale = 10.0f;
bool wireframe = false;

uint32_t maxNumberOfSamples = 4096;
uint32_t numberOfSamples = 4;
bool restartSampling = true;

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

    CameraUBO camUBO = {};
    camUBO.numberOfBounces = 4;
    camUBO.totalNumberOfSamples = 0;
    camUBO.numberOfSamples = 0;
    /*
    ubo.aperture = 0.5f;
    ubo.focusDistance = 1.0f;
    ubo.totalNumberOfSamples = 0;
    ubo.numberOfSamples = 8;
    ubo.numberOfBounces = 2;
    ubo.randomSeed = 1;
    ubo.gammaCorrection = false;
    ubo.hasSky = false;
    */

    std::vector<estun::UniformBuffer<CameraUBO>> camUBs(context->GetSwapChain()->GetImageViews().size());
    std::vector<std::shared_ptr<estun::Model>> models;

    float box_scale = 3.0f;
    models.push_back(std::make_shared<estun::Model>(CornellBox::CreateCornellBox(box_scale)));
    models.back()->Transform(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f * box_scale, -0.5f * box_scale, 0.0f)));

    estun::Material colorMaterial = estun::Material::Lambertian(glm::vec3(0.5f, 0.5f, 0.5f), -1);

    models.push_back(std::make_shared<estun::Model>(estun::Model::CreateBox(glm::vec3(0.0f), glm::vec3(0.8f, 1.5f, 0.8f), colorMaterial)));
    glm::mat4 transform(1.0f);
    transform = glm::rotate(transform, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(-0.2f * box_scale, -0.5f * box_scale, -2.0f));
    models.back()->Transform(transform);
    models.push_back(std::make_shared<estun::Model>(estun::Model::CreateSphere(glm::vec3(0.0f), 0.4f, colorMaterial)));
    transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f * box_scale, -0.5f * box_scale + 0.4f, -2.0f));
    models.back()->Transform(transform);

    std::vector<estun::Texture> textures;

    // If there are no texture, add a dummy one. It makes the pipeline setup a lot easier.
    if (textures.empty())
    {
        textures.push_back(estun::Texture("assets/textures/white.png"));
    }

    std::vector<estun::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<estun::Material> materials;
    std::vector<glm::uvec2> offsets;

    for (const auto &model : models)
    {
        const auto indexOffset = static_cast<uint32_t>(indices.size());
        const auto vertexOffset = static_cast<uint32_t>(vertices.size());
        const auto materialOffset = static_cast<uint32_t>(materials.size());

        offsets.emplace_back(indexOffset, vertexOffset);

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
    std::shared_ptr<estun::StorageBuffer<glm::uvec2>> offsetBuffer = std::make_shared<estun::StorageBuffer<glm::uvec2>>(offsets);

    std::vector<std::shared_ptr<estun::BLAS>> blases = estun::BLAS::CreateBlases(models, VB, IB);
    std::shared_ptr<estun::TLAS> tlas = std::make_shared<estun::TLAS>(blases);

    auto extent = estun::ContextLocator::GetSwapChain()->GetExtent();
    std::shared_ptr<estun::Image> storeImage = estun::Image::CreateStorageImage(extent.width, extent.height, estun::ContextLocator::GetSwapChain()->GetFormat());
    storeImage->ToLayout(VK_IMAGE_LAYOUT_GENERAL);
    std::shared_ptr<estun::Image> accumulationImage = estun::Image::CreateStorageImage(extent.width, extent.height, VK_FORMAT_R32G32B32A32_SFLOAT);
    accumulationImage->ToLayout(VK_IMAGE_LAYOUT_GENERAL);

    std::vector<estun::DescriptorBinding> descriptorBindings = {
        estun::DescriptorBinding::AccelerationStructure(0, tlas, VK_SHADER_STAGE_RAYGEN_BIT_KHR),
        estun::DescriptorBinding::StorageImage(1, storeImage, VK_SHADER_STAGE_RAYGEN_BIT_KHR),
        estun::DescriptorBinding::StorageImage(2, accumulationImage, VK_SHADER_STAGE_RAYGEN_BIT_KHR),
        estun::DescriptorBinding::Uniform(3, camUBs, VK_SHADER_STAGE_RAYGEN_BIT_KHR),
        estun::DescriptorBinding::Storage(4, VB, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),
        estun::DescriptorBinding::Storage(5, IB, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),
        estun::DescriptorBinding::Storage(6, materialBuffer, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),
        estun::DescriptorBinding::Storage(7, offsetBuffer, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),
        estun::DescriptorBinding::Textures(8, textures, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)};

    std::shared_ptr<estun::Descriptor> descriptor = std::make_shared<estun::Descriptor>(descriptorBindings, context->GetSwapChain()->GetImageViews().size());
    descriptorBindings.clear();

    std::shared_ptr<estun::RayTracingRender> render = context->CreateRayTracingRender();

    std::shared_ptr<estun::RayTracingPipeline> pipeline = render->CreatePipeline(
        {{"assets/shaders/main.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR},
         {"assets/shaders/main.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR},
         {"assets/shaders/main.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR}},
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
        if (restartSampling)
        {
            camUBO.numberOfSamples = 0;
            camUBO.totalNumberOfSamples = 0;
            restartSampling = false;
        }

        camUBO.numberOfSamples = glm::clamp(maxNumberOfSamples - camUBO.totalNumberOfSamples, 0u, numberOfSamples);
        camUBO.totalNumberOfSamples += camUBO.numberOfSamples;

        float currFrame = window->Time();
        deltaTime = lastFrame - currFrame;
        lastFrame = currFrame;
        glfwPollEvents();

        context->StartDraw();

        camUBO.camPos = glm::vec4(camera.Position, 1.0f);
        camUBO.camDir = glm::vec4(camera.Front, 1.0f);
        camUBO.camUp = glm::vec4(camera.Up, 1.0f);
        camUBO.camSide = glm::vec4(camera.Right, 1.0f);
        camUBO.camNearFarFov = glm::vec4(0.01f, 100.0f, glm::radians(camera.Zoom), 1.0f);

        camUBs[context->GetImageIndex()].SetValue(camUBO);

        context->SubmitDraw();
    }

    pipeline.reset();
    render.reset();
    context->Clear();
    blases.clear();
    tlas.reset();
    shaderBindingTable.reset();
    storeImage.reset();
    accumulationImage.reset();
    camUBs.clear();
    VB.reset();
    IB.reset();
    materialBuffer.reset();
    offsetBuffer.reset();
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
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
        if (!cursor)
            restartSampling = true;
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT) && !cursor)
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (!cursor)
            restartSampling = true;
    }
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT) && !cursor)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
        if (!cursor)
            restartSampling = true;
    }
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT) && !cursor)
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
        if (!cursor)
            restartSampling = true;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        wireframe = !wireframe;
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        window->ToggleCursor(!cursor);
        cursor = !cursor;
    }
}

void mouse_callback(double xpos, double ypos)
{
    if (!cursor)
        restartSampling = true;

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

    if (!cursor)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(double xoffset, double yoffset)
{
    if (!cursor)
        restartSampling = true;

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
