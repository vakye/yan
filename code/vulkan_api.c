
#pragma once

typedef struct
{
    #define VulkanDeclare(Name) \
        PFN_vk##Name Name

    // NOTE(vak): Non-instance functions

    VulkanDeclare(GetInstanceProcAddr);
    VulkanDeclare(CreateInstance);
    VulkanDeclare(EnumerateInstanceVersion);

    // NOTE(vak): Instance functions

    VulkanDeclare(CreateWaylandSurfaceKHR);
    VulkanDeclare(EnumeratePhysicalDevices);
    VulkanDeclare(GetPhysicalDeviceProperties);
    VulkanDeclare(GetPhysicalDeviceQueueFamilyProperties);
    VulkanDeclare(GetPhysicalDeviceMemoryProperties);
    VulkanDeclare(GetPhysicalDeviceSurfaceFormatsKHR);
    VulkanDeclare(GetPhysicalDeviceSurfacePresentModesKHR);
    VulkanDeclare(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    VulkanDeclare(CreateDevice);
    VulkanDeclare(GetDeviceQueue);
    VulkanDeclare(CreateCommandPool);
    VulkanDeclare(AllocateCommandBuffers);
    VulkanDeclare(CreateSemaphore);
    VulkanDeclare(CreateImageView);
    VulkanDeclare(DestroyImageView);
    VulkanDeclare(CreateDescriptorSetLayout);
    VulkanDeclare(CreatePipelineLayout);
    VulkanDeclare(CreateShaderModule);
    VulkanDeclare(DestroyShaderModule);
    VulkanDeclare(CreateGraphicsPipelines);
    VulkanDeclare(AllocateMemory);
    VulkanDeclare(MapMemory);
    VulkanDeclare(CreateBuffer);
    VulkanDeclare(GetBufferMemoryRequirements);
    VulkanDeclare(BindBufferMemory);
    VulkanDeclare(CreateSwapchainKHR);
    VulkanDeclare(DestroySwapchainKHR);
    VulkanDeclare(GetSwapchainImagesKHR);
    VulkanDeclare(AcquireNextImageKHR);
    VulkanDeclare(ResetCommandBuffer);
    VulkanDeclare(BeginCommandBuffer);
    VulkanDeclare(EndCommandBuffer);
    VulkanDeclare(CmdBeginRendering);
    VulkanDeclare(CmdEndRendering);
    VulkanDeclare(CmdBindPipeline);
    VulkanDeclare(CmdSetViewport);
    VulkanDeclare(CmdSetScissor);
    VulkanDeclare(CmdPushDescriptorSet);
    VulkanDeclare(CmdPushConstants);
    VulkanDeclare(CmdDraw);
    VulkanDeclare(CmdPipelineBarrier);
    VulkanDeclare(QueueSubmit);
    VulkanDeclare(QueuePresentKHR);
    VulkanDeclare(DeviceWaitIdle);

    #undef VulkanDeclare
} vulkan_api;

local b32 VulkanLoadNonInstanceAPI(
    vulkan_api* API,
    void* vkGetInstanceProcAddr
)
{
    if (!vkGetInstanceProcAddr)
        return (false);

    API->GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)
        vkGetInstanceProcAddr;

    API->CreateInstance = (PFN_vkCreateInstance)
        API->GetInstanceProcAddr(0, "vkCreateInstance");

    API->EnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)
        API->GetInstanceProcAddr(0, "vkEnumerateInstanceVersion");

    if (!API->CreateInstance) return (false);
    if (!API->EnumerateInstanceVersion) return (false);

    return (true);
}

local b32 VulkanLoadInstanceAPI(
    vulkan_api* API,
    VkInstance Instance,
    b32 Wayland
)
{
    #define VulkanLoad(Name) \
        API->Name = (PFN_vk##Name)API->GetInstanceProcAddr(Instance, "vk" #Name); \
        if (!API->Name) \
            return (false)

    if (Wayland)
    {
        VulkanLoad(CreateWaylandSurfaceKHR);
    }

    VulkanLoad(CreateWaylandSurfaceKHR);
    VulkanLoad(EnumeratePhysicalDevices);
    VulkanLoad(GetPhysicalDeviceProperties);
    VulkanLoad(GetPhysicalDeviceQueueFamilyProperties);
    VulkanLoad(GetPhysicalDeviceMemoryProperties);
    VulkanLoad(GetPhysicalDeviceSurfaceFormatsKHR);
    VulkanLoad(GetPhysicalDeviceSurfacePresentModesKHR);
    VulkanLoad(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    VulkanLoad(CreateDevice);
    VulkanLoad(GetDeviceQueue);
    VulkanLoad(CreateCommandPool);
    VulkanLoad(AllocateCommandBuffers);
    VulkanLoad(CreateSemaphore);
    VulkanLoad(CreateImageView);
    VulkanLoad(DestroyImageView);
    VulkanLoad(CreateDescriptorSetLayout);
    VulkanLoad(CreatePipelineLayout);
    VulkanLoad(CreateShaderModule);
    VulkanLoad(DestroyShaderModule);
    VulkanLoad(CreateGraphicsPipelines);
    VulkanLoad(AllocateMemory);
    VulkanLoad(MapMemory);
    VulkanLoad(CreateBuffer);
    VulkanLoad(GetBufferMemoryRequirements);
    VulkanLoad(BindBufferMemory);
    VulkanLoad(CreateSwapchainKHR);
    VulkanLoad(DestroySwapchainKHR);
    VulkanLoad(GetSwapchainImagesKHR);
    VulkanLoad(AcquireNextImageKHR);
    VulkanLoad(ResetCommandBuffer);
    VulkanLoad(BeginCommandBuffer);
    VulkanLoad(EndCommandBuffer);
    VulkanLoad(CmdBeginRendering);
    VulkanLoad(CmdEndRendering);
    VulkanLoad(CmdBindPipeline);
    VulkanLoad(CmdSetViewport);
    VulkanLoad(CmdSetScissor);
    VulkanLoad(CmdPushDescriptorSet);
    VulkanLoad(CmdPushConstants);
    VulkanLoad(CmdDraw);
    VulkanLoad(CmdPipelineBarrier);
    VulkanLoad(QueueSubmit);
    VulkanLoad(QueuePresentKHR);
    VulkanLoad(DeviceWaitIdle);

    #undef VulkanLoad

    return (true);
}

