#include "renderer/vulkan/vulkan_context.h"
#include "renderer/material/vulkan_shader_manager.h"
#include <iostream>

namespace estun {
    VulkanContext* VulkanContextLocator::currContext = nullptr;
    
	VulkanContext::VulkanContext(GLFWwindow* windowHandle, GameInfo* appGameInfo)
	{
        gameInfo = appGameInfo;
        if (!windowHandle)
        {
		    ES_CORE_ASSERT("Window handle is null!");
        }
		Init(windowHandle);
	}

    VulkanContext::~VulkanContext()
    {
        Shutdown();
    }
	
	void VulkanContext::Init(GLFWwindow* windowHandle)
	{
        ES_CORE_INFO("start vulkan init");
	    instance       = new VulkanInstance(gameInfo->name.c_str(), gameInfo->version);
        ES_CORE_INFO("instance done");
        surface        = new VulkanSurface(instance, windowHandle);
        ES_CORE_INFO("surface done");
        device         = new VulkanDevice(instance, surface);
        ES_CORE_INFO("device done");
        VulkanDeviceLocator::Provide(device);

        swapChain      = new VulkanSwapChain(surface, device->GetQueueFamilyIndices(), gameInfo->width, gameInfo->height);
        ES_CORE_INFO("swap chain done");
        imageView      = new VulkanImageView(swapChain);
        ES_CORE_INFO("image view done");
        renderPass     = new VulkanRenderPass(*swapChain->GetSwapChainImageFormat(), device->GetMsaaSamples());
        ES_CORE_INFO("render pass done");
        
        commandPool    = new VulkanCommandPool();
        ES_CORE_INFO("command pool done");
        colorResources = new VulkanColorResources(swapChain, device->GetMsaaSamples());
        ES_CORE_INFO("color resources done");
        depthResources = new VulkanDepthResources(swapChain->GetSwapChainExtent(), device->GetMsaaSamples());
        ES_CORE_INFO("depth resources done");
        framebuffers   = new VulkanFramebuffers(
            imageView,
            renderPass, 
            swapChain->GetSwapChainExtent(), 
            depthResources->GetDepthImageView(), 
            colorResources->GetColorImageView());

        ES_CORE_INFO("frame buffers done");
       
        commandBuffers = new VulkanCommandBuffers(commandPool, swapChain->GetImageCount());
        ES_CORE_INFO("command buffers done");
        semaphores     = new VulkanSemaphoresManager(maxFramesInFlight, swapChain->GetImageCount());
        ES_CORE_INFO("semaphores done");
        currentFrame = 0;
    }  
    
    void VulkanContext::Shutdown()
    {
        delete semaphores;
        CleanUpSwapChain();
        ShaderManager::GetInstance()->CleanUp();
        delete commandPool;
        delete device;
        VulkanDeviceLocator::Provide(nullptr);
        surface->Delete(instance);
        delete surface;
        delete instance;
    }

    void VulkanContext::RecreateSwapChain()
    {
        vkDeviceWaitIdle(*device->GetLogicalDevice());

        CleanUpSwapChain();

        swapChain      = new VulkanSwapChain(surface, device->GetQueueFamilyIndices(), gameInfo->width, gameInfo->height);
        imageView      = new VulkanImageView(swapChain);
        renderPass     = new VulkanRenderPass(*swapChain->GetSwapChainImageFormat(), device->GetMsaaSamples());
        
        VulkanMaterialPoolLocator::RebuildPipelines();

        colorResources = new VulkanColorResources(swapChain, device->GetMsaaSamples());
        depthResources = new VulkanDepthResources(swapChain->GetSwapChainExtent(), device->GetMsaaSamples());
        framebuffers   = new VulkanFramebuffers(imageView, renderPass, swapChain->GetSwapChainExtent(), depthResources->GetDepthImageView(), colorResources->GetColorImageView());
        
        commandBuffers = new VulkanCommandBuffers(commandPool, swapChain->GetImageCount());
    }

    void VulkanContext::CleanUpSwapChain()
    {
        delete depthResources;
        delete colorResources;
        delete framebuffers;
        delete commandBuffers;
        delete imageView;
        delete swapChain;
        delete renderPass;
    }
    
    void VulkanContext::Submit(VulkanVertexBuffer* vbo, VulkanIndexBuffer* ibo, VulkanMaterial* material)
    {
        vkDeviceWaitIdle(*device->GetLogicalDevice());

        //commandBuffers->ResetCommandBuffers();
        commandBuffers->InitCommandBuffers(renderPass, swapChain->GetSwapChainExtent(), framebuffers);

        commandBuffers->BindShader(material->GetPipeline(), material->GetDescriptorSets());
        commandBuffers->LoadDraw(vbo, ibo);

        commandBuffers->CloseCommandBuffers();
    }


    void VulkanContext::SubmitCompute(uint32_t x, uint32_t y, VulkanMaterial* material)
    {
        vkDeviceWaitIdle(*device->GetLogicalDevice());

        //commandBuffers->ResetCommandBuffers();
        commandBuffers->BeginCommandBuffers();

        commandBuffers->BindShader(material->GetComputePipeline(), material->GetComputeDescriptorSets());
        commandBuffers->LoadCompute(x, y);

        commandBuffers->EndCommandBuffers();
    }

    void VulkanContext::StartCompute()
    {
		VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};

        //VkSemaphore waitSemaphores[] = {(*semaphores->GetComputeAvailableSemaphores())};
        VkSemaphore waitSemaphores[] = {(*semaphores->GetImageAvailableSemaphores())[currentFrame]};

		if (!firstCompute) {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
		} else {
			firstCompute = false;
		}

		submitInfo.signalSemaphoreCount = 1;
        VkSemaphore signalSemaphores[] = {(*semaphores->GetComputeFinishedSemaphores())};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(*commandBuffers->GetCommandBuffersVector())[0];

        if (vkQueueSubmit(*device->GetComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to submit compute command buffer");
        }
    }

    void VulkanContext::PrepareFrame()
    {
        vkWaitForFences(*device->GetLogicalDevice(), 1, &(*semaphores->GetInFlightFences())[currentFrame], VK_TRUE, UINT64_MAX);
        VkResult result = vkAcquireNextImageKHR(
            *device->GetLogicalDevice(),
            *swapChain->GetSwapChain(),
            UINT64_MAX,
            (*semaphores->GetImageAvailableSemaphores())[currentFrame],
            VK_NULL_HANDLE, &imageIndex
        );
        if (result == VK_SUCCESS)
        {
            return;
        }
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) 
        {
		    RecreateSwapChain();
	    }
	    else 
        {
            ES_CORE_ASSERT("Cant aquire next image");
	    }
    }

    void VulkanContext::SubmitFrame()
    {
        if ((*semaphores->GetImagesInFlight())[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(*device->GetLogicalDevice(), 1, &(*semaphores->GetInFlightFences())[currentFrame], VK_TRUE, UINT64_MAX);
        }
        (*semaphores->GetImagesInFlight())[imageIndex] = (*semaphores->GetInFlightFences())[currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        //VkSemaphore waitSemaphores[] = {(*semaphores->GetImageAvailableSemaphores())[currentFrame]};
        VkSemaphore waitSemaphores[] = {(*semaphores->GetComputeFinishedSemaphores())};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(*commandBuffers->GetCommandBuffersVector())[imageIndex];

        VkSemaphore signalSemaphores[] = {(*semaphores->GetRenderFinishedSemaphores())[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(*device->GetLogicalDevice(), 1, &(*semaphores->GetInFlightFences())[currentFrame]);

        if (vkQueueSubmit(*device->GetGraphicsQueue(), 1, &submitInfo, (*semaphores->GetInFlightFences())[currentFrame]) != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to submit draw command buffer");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {*swapChain->GetSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(*device->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) 
        {
            framebufferResized = false;
            RecreateSwapChain();
        } 
        else if (result != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to present swap chain image");
        }

        currentFrame = (currentFrame + 1) % maxFramesInFlight;   
    }

    void VulkanContext::ResizeFramebuffers()
    {
        framebufferResized = true;
    }

    void VulkanContext::FreeComandBuffers()
    {
        commandBuffers->FreeCommandBuffers();
    }

    void VulkanContext::EndDraw()
    {
        vkDeviceWaitIdle(*device->GetLogicalDevice());
    }

/*
    void VulkanContext::SwapBuffers()
    {
        //glfwSwapBuffers(m_WindowHandle->m_Window);
    }

	void VulkanContext::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
        //int width = 0, height = 0;
        //glfwGetFramebufferSize(window, &width, &height);
        //while (width == 0 || height == 0) {
        //    glfwGetFramebufferSize(window, &width, &height);
        //    glfwWaitEvents();
        //}
		//glViewport(x, y, width, height);
	}

	void VulkanContext::SetClearColor(const glm::vec4& color)
	{
		//glClearColor(color.r, color.g, color.b, color.a);
	}

	void VulkanContext::Clear()
	{
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void VulkanContext::DrawIndexed(const ref<VertexArray>& vertexArray)
	{
		//glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		//glBindTexture(GL_TEXTURE_2D, 0);
	}

*/
}
