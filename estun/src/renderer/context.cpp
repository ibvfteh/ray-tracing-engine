#include "renderer/context.h"
#include "renderer/context/image_memory_barier.h"

estun::Context::Context(GLFWwindow *windowHandle, GameInfo *gameInfo)
{
    if (!windowHandle)
    {
        ES_CORE_ASSERT("Window handle is null!");
    }

    gameInfo_ = gameInfo;

    ES_CORE_INFO("Start vulkan init");
    instance_.reset(new Instance(gameInfo_->name_.c_str(), gameInfo_->version_));
    ES_CORE_INFO("Instance done");
    surface_.reset(new Surface(instance_.get(), windowHandle));
    ES_CORE_INFO("Surface done");
    device_.reset(new Device(instance_.get(), surface_.get()));
    DeviceLocator::Provide(device_.get());
    dynamicFunctions_.reset(new DynamicFunctions());
    FunctionsLocator::Provide(dynamicFunctions_.get());
    RayTracingPropertiesLocator::Provide(new RayTracingProperties());
    ES_CORE_INFO("Device done");

    graphicsCommandPool_.reset(new CommandPool(Graphics));
    ES_CORE_INFO("Graphics command pool done");
    computeCommandPool_.reset(new CommandPool(Compute));
    ES_CORE_INFO("Compute command pool done");
    transferCommandPool_.reset(new CommandPool(Transfer));
    ES_CORE_INFO("Transfer command pool done");
    CommandPoolLocator::Provide(graphicsCommandPool_.get(), computeCommandPool_.get(), transferCommandPool_.get());

    msaa_ = VK_SAMPLE_COUNT_1_BIT;
    if (gameInfo_->msaa_)
        msaa_ = device_->GetMsaaSamples();

    CreateSwapChain();
}

estun::Context::~Context()
{
    DeleteSwapChain();

    transferCommandPool_.reset();
    computeCommandPool_.reset();
    graphicsCommandPool_.reset();
    device_.reset();
    surface_->Delete(instance_.get());
    surface_.reset();
    instance_.reset();
}

void estun::Context::CreateSwapChain()
{
    swapChain_.reset(new SwapChain(surface_.get(), gameInfo_->width_, gameInfo_->height_, gameInfo_->vsync_));
    ES_CORE_INFO("Swap chain done");

    for (size_t i = 0; i != swapChain_->GetImageViews().size(); ++i)
    {
        imageAvailableSemaphores_.emplace_back();
        renderFinishedSemaphores_.emplace_back();
        computeFinishedSemaphores_.emplace_back();
        inFlightFences_.emplace_back(true);
    }
    ES_CORE_INFO("Semaphores done");
}

void estun::Context::DeleteSwapChain()
{
    inFlightFences_.clear();
    renderFinishedSemaphores_.clear();
    imageAvailableSemaphores_.clear();
    computeFinishedSemaphores_.clear();
    swapChain_.reset();
}

void estun::Context::RecreateSwapChain()
{
    device_->WaitIdle();
    DeleteSwapChain();
    CreateSwapChain();
    for (auto &render : graphicsRenders_)
    {
        render->Recreate();
    }
}

void estun::Context::Clear()
{
    device_->WaitIdle();
    graphicsRenders_.clear();
    computeRenders_.clear();
    rayTracingRenders_.clear();
    DeleteSwapChain();
}

std::shared_ptr<estun::GraphicsRender> estun::Context::CreateGraphicsRender(bool toDefault)
{
    std::shared_ptr<GraphicsRender> render = std::make_shared<GraphicsRender>(toDefault);
    graphicsRenders_.push_back(render);
    return render;
}

std::shared_ptr<estun::ComputeRender> estun::Context::CreateComputeRender()
{
    std::shared_ptr<ComputeRender> render = std::make_shared<ComputeRender>();
    computeRenders_.push_back(render);
    return render;
}

std::shared_ptr<estun::RayTracingRender> estun::Context::CreateRayTracingRender()
{
    std::shared_ptr<RayTracingRender> render = std::make_shared<RayTracingRender>();
    rayTracingRenders_.push_back(render);
    return render;
}

void estun::Context::StartDraw()
{
    const auto noTimeout = std::numeric_limits<uint64_t>::max();

    auto &inFlightFence = inFlightFences_[currentFrame_];
    const auto imageAvailableSemaphore = imageAvailableSemaphores_[currentFrame_].GetSemaphore();

    inFlightFence.Wait(noTimeout);

    auto result = vkAcquireNextImageKHR(device_->GetLogicalDevice(), swapChain_->GetSwapChain(), noTimeout, imageAvailableSemaphore, nullptr, &imageIndex_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        ES_CORE_ASSERT(std::string("Failed to acquire next image (") + std::string(")"));
    }
}

void estun::Context::SubmitDraw()
{

    const auto imageAvailableSemaphore = imageAvailableSemaphores_[currentFrame_].GetSemaphore();
    const auto renderFinishedSemaphore = renderFinishedSemaphores_[currentFrame_].GetSemaphore();
    const auto computeFinishedSemaphore = computeFinishedSemaphores_[currentFrame_].GetSemaphore();

    std::vector<VkCommandBuffer> computeCommandBuffers;
    for (auto &render : computeRenders_)
    {
        computeCommandBuffers.push_back(render->GetCurrCommandBuffer());
    };

    VkSemaphore computeWaitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags computeWaitStages[] = {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
    VkSemaphore computeSignalSemaphores[] = {computeFinishedSemaphore};

    VkSubmitInfo computeSubmitInfo = {};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmitInfo.waitSemaphoreCount = 1;
    computeSubmitInfo.pWaitSemaphores = computeWaitSemaphores;
    computeSubmitInfo.pWaitDstStageMask = computeWaitStages;
    computeSubmitInfo.commandBufferCount = static_cast<uint32_t>(computeCommandBuffers.size());
    computeSubmitInfo.pCommandBuffers = computeCommandBuffers.data();
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores = computeSignalSemaphores;

    VK_CHECK_RESULT(vkQueueSubmit(device_->GetComputeQueue(), 1, &computeSubmitInfo, nullptr), "submit compute command buffers");

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    auto &inFlightFence = inFlightFences_[currentFrame_];

    std::vector<VkCommandBuffer> commandBuffers;
    for (auto &render : graphicsRenders_)
    {
        commandBuffers.push_back(render->GetCurrCommandBuffer());
    }
    VkSemaphore waitSemaphores[] = {computeFinishedSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    submitInfo.pCommandBuffers = commandBuffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    inFlightFence.Reset();

    VK_CHECK_RESULT(vkQueueSubmit(device_->GetGraphicsQueue(), 1, &submitInfo, inFlightFence.GetFence()), "submit draw command buffers");

    VkSwapchainKHR swapChains[] = {swapChain_->GetSwapChain()};
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex_;
    presentInfo.pResults = nullptr; // Optional

    auto result = vkQueuePresentKHR(device_->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS)
    {
        ES_CORE_ASSERT(std::string("failed to present next image (") + std::string(")"));
    }

    currentFrame_ = (currentFrame_ + 1) % inFlightFences_.size();
}

estun::Context *estun::ContextLocator::currContext = nullptr;

estun::Context *estun::ContextLocator::GetContext()
{
    if (currContext == nullptr)
    {
        ES_CORE_ASSERT("Failed to request vulkan context");
    }
    return currContext;
}

estun::SwapChain *estun::ContextLocator::GetSwapChain()
{
    if (currContext == nullptr)
    {
        ES_CORE_ASSERT("Failed to request vulkan context");
    }
    return currContext->GetSwapChain();
}

uint32_t estun::ContextLocator::GetImageIndex()
{
    if (currContext == nullptr)
    {
        ES_CORE_ASSERT("Failed to request vulkan context");
    }
    return currContext->GetImageIndex();
}