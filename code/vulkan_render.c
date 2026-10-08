
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>

typedef enum
{
    VulkanSurfaceKind_Nil = 0,
    VulkanSurfaceKind_Wayland,
} vulkan_surface_kind;

typedef struct
{
    void* vkGetInstanceProcAddr;

    vulkan_surface_kind SurfaceKind;

    // NOTE(vak): VulkanSurfaceKind_Wayland

    void* WaylandDisplay; // NOTE(vak): wl_display
    void* WaylandSurface; // NOTE(vak): wl_surface
} vulkan_setup_info;

typedef struct
{
    u32 TargetSizeX;
    u32 TargetSizeY;
} vulkan_render_info;

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
    VulkanDeclare(CreatePipelineLayout);
    VulkanDeclare(CreateShaderModule);
    VulkanDeclare(DestroyShaderModule);
    VulkanDeclare(CreateGraphicsPipelines);
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
    VulkanDeclare(CmdDraw);
    VulkanDeclare(CmdPipelineBarrier);
    VulkanDeclare(QueueSubmit);
    VulkanDeclare(QueuePresentKHR);
    VulkanDeclare(DeviceWaitIdle);

    #undef VulkanDeclare
} vulkan_api;

typedef struct
{
    u32                     VersionOfAPI;
    VkInstance              Instance;
    VkSurfaceKHR            Surface;
    VkPhysicalDevice        PhysicalDevice;
    u32                     QueueFamilyIndex;
    VkDevice                Device;
    VkQueue                 Queue;
    VkCommandPool           CommandPool;
    VkCommandBuffer         CommandBuffer;
    VkSemaphore             AcquireSemaphore;
    VkSemaphore             SubmitSemaphore;
    VkSurfaceFormatKHR      SwapchainFormat;
    VkPresentModeKHR        PresentMode;
    VkPipelineLayout        PipelineLayout;
    VkPipeline              Pipeline;
    VkExtent2D              SwapchainExtent;
    VkSwapchainKHR          Swapchain;
    u32                     SwapchainImageCount;
    VkImage                 SwapchainImages[16];
    VkImageView             SwapchainImageViews[16];

    vulkan_api              API;
} vulkan_state;

local b32   VulkanSetup     (vulkan_state* Vulkan, vulkan_setup_info* Info);
local void  VulkanRender    (vulkan_state* Vulkan, vulkan_render_info* Info);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================


local b32 VulkanSetup(vulkan_state* Vulkan, vulkan_setup_info* Info)
{
    #define VulkanReturnOnError(VulkanCall) if ((VulkanCall) != VK_SUCCESS) return (false)

    if (!Info->vkGetInstanceProcAddr) return (false);

    vulkan_api* API = &Vulkan->API;

    // NOTE(vak): Load non-instance functions
    {
        API->GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)
            Info->vkGetInstanceProcAddr;

        API->CreateInstance = (PFN_vkCreateInstance)
            API->GetInstanceProcAddr(0, "vkCreateInstance");

        API->EnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)
            API->GetInstanceProcAddr(0, "vkEnumerateInstanceVersion");

        if (!API->CreateInstance) return (false);
        if (!API->EnumerateInstanceVersion) return (false);
    }

    // NOTE(vak): Create instance
    {
        Vulkan->VersionOfAPI = VK_API_VERSION_1_4;

        u32 InstanceVersion = 0;
        VulkanReturnOnError(
            API->EnumerateInstanceVersion(&InstanceVersion)
        );

        if (InstanceVersion < Vulkan->VersionOfAPI)
            return (false);

        const char* Layers[] =
        {
            "VK_LAYER_KHRONOS_validation",
        };

        const char* Extensions[2] =
        {
            "VK_KHR_surface",
            0,
        };

        switch (Info->SurfaceKind)
        {
            default: return (false);

            case VulkanSurfaceKind_Wayland: Extensions[1] = "VK_KHR_wayland_surface"; break;
        }

        VkInstanceCreateInfo InstanceInfo =
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &(VkApplicationInfo)
            {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = "yan",
                .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                .pEngineName = "yan",
                .engineVersion = VK_MAKE_VERSION(0, 0, 1),
                .apiVersion = Vulkan->VersionOfAPI,
            },
            .enabledLayerCount = ArrayCount(Layers),
            .ppEnabledLayerNames = Layers,
            .enabledExtensionCount = ArrayCount(Extensions),
            .ppEnabledExtensionNames = Extensions,
        };

        VulkanReturnOnError(API->CreateInstance(
            &InstanceInfo, 0, &Vulkan->Instance
        ));
    }

    // NOTE(vak): Load instance functions
    {
        #define VulkanLoad(Name) \
            API->Name = (PFN_vk##Name)API->GetInstanceProcAddr(Vulkan->Instance, "vk" #Name); \
            if (!API->Name) \
                return (false)

        switch (Info->SurfaceKind)
        {
            default: return (false);

            case VulkanSurfaceKind_Wayland:
                VulkanLoad(CreateWaylandSurfaceKHR);
                break;
        }

        VulkanLoad(CreateWaylandSurfaceKHR);
        VulkanLoad(EnumeratePhysicalDevices);
        VulkanLoad(GetPhysicalDeviceProperties);
        VulkanLoad(GetPhysicalDeviceQueueFamilyProperties);
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
        VulkanLoad(CreatePipelineLayout);
        VulkanLoad(CreateShaderModule);
        VulkanLoad(DestroyShaderModule);
        VulkanLoad(CreateGraphicsPipelines);
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
        VulkanLoad(CmdDraw);
        VulkanLoad(CmdPipelineBarrier);
        VulkanLoad(QueueSubmit);
        VulkanLoad(QueuePresentKHR);
        VulkanLoad(DeviceWaitIdle);

        #undef VulkanLoad
    }

    // NOTE(vak): Create surface
    {
        switch (Info->SurfaceKind)
        {
            default: return (false);

            case VulkanSurfaceKind_Wayland:
            {
                if (!Info->WaylandDisplay) return (false);
                if (!Info->WaylandSurface) return (false);

                VkWaylandSurfaceCreateInfoKHR SurfaceInfo =
                {
                    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
                    .display = Info->WaylandDisplay,
                    .surface = Info->WaylandSurface,
                };

                VulkanReturnOnError(API->CreateWaylandSurfaceKHR(
                    Vulkan->Instance, &SurfaceInfo, 0, &Vulkan->Surface
                ));
            } break;
        }
    }

    // NOTE(vak): Physical device
    {
        VkPhysicalDevice Array[64] = {0};
        u32 Count = ArrayCount(Array);

        VulkanReturnOnError(API->EnumeratePhysicalDevices(
            Vulkan->Instance,
            &Count,
            Array
        ));

        VkPhysicalDevice Preferred = {0};
        VkPhysicalDevice Fallback = {0};

        for (usize Index = 0; Index < Count; Index++)
        {
            VkPhysicalDevice GPU = Array[Index];

            VkPhysicalDeviceProperties Properties = {0};
            API->GetPhysicalDeviceProperties(GPU, &Properties);

            if (Properties.apiVersion < Vulkan->VersionOfAPI)
                continue;

            if (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                if (!Preferred) Preferred = GPU;
            }
            else
            {
                if (!Fallback) Fallback = GPU;
            }
        }

        Vulkan->PhysicalDevice = (Preferred) ? (Preferred) : (Fallback);
        if (!Vulkan->PhysicalDevice) return (false);
    }

    // NOTE(vak): Queue family
    {
        Vulkan->QueueFamilyIndex = U32_MAX;

        VkQueueFamilyProperties Array[64] = {0};
        u32 Count = ArrayCount(Array);

        API->GetPhysicalDeviceQueueFamilyProperties(
            Vulkan->PhysicalDevice,
            &Count,
            Array
        );

        for (u32 Index = 0; Index < Count; Index++)
        {
            VkQueueFamilyProperties* Properties = Array + Index;

            VkQueueFlags RequiredFlags =
                VK_QUEUE_GRAPHICS_BIT |
                VK_QUEUE_TRANSFER_BIT |
                VK_QUEUE_COMPUTE_BIT;

            if ((Properties->queueFlags & RequiredFlags) == RequiredFlags)
            {
                Vulkan->QueueFamilyIndex = Index;
                break;
            }
        }

        if (Vulkan->QueueFamilyIndex == U32_MAX)
            return (false);
    }

    // NOTE(vak): Device
    {
        const char* Extensions[] =
        {
            "VK_KHR_swapchain",
        };

        VkPhysicalDeviceVulkan13Features Vulkan13Features =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .dynamicRendering = true,
        };

        VkDeviceCreateInfo DeviceInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &Vulkan13Features,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &(VkDeviceQueueCreateInfo)
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = Vulkan->QueueFamilyIndex,
                .queueCount = 1,
                .pQueuePriorities = (f32[1]){1.0f},
            },
            .enabledExtensionCount = ArrayCount(Extensions),
            .ppEnabledExtensionNames = Extensions,
        };

        VulkanReturnOnError(API->CreateDevice(
            Vulkan->PhysicalDevice, &DeviceInfo, 0, &Vulkan->Device
        ));
    }

    // NOTE(vak): Queue
    {
        API->GetDeviceQueue(
            Vulkan->Device, Vulkan->QueueFamilyIndex, 0, &Vulkan->Queue
        );
    }

    // NOTE(vak): Command pool
    {
        VkCommandPoolCreateInfo PoolInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = Vulkan->QueueFamilyIndex,
        };

        VulkanReturnOnError(API->CreateCommandPool(
            Vulkan->Device, &PoolInfo, 0, &Vulkan->CommandPool
        ));
    }

    // NOTE(vak): Command buffer
    {
        VkCommandBufferAllocateInfo AllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = Vulkan->CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VulkanReturnOnError(API->AllocateCommandBuffers(
            Vulkan->Device, &AllocateInfo, &Vulkan->CommandBuffer
        ));
    }

    // NOTE(vak): Semaphores
    {
        VkSemaphoreCreateInfo SemaphoreInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VulkanReturnOnError(API->CreateSemaphore(
            Vulkan->Device, &SemaphoreInfo, 0, &Vulkan->AcquireSemaphore
        ));

        VulkanReturnOnError(API->CreateSemaphore(
            Vulkan->Device, &SemaphoreInfo, 0, &Vulkan->SubmitSemaphore
        ));
    }

    // NOTE(vak): Swapchain format
    {
        VkSurfaceFormatKHR Array[512] = {0};
        u32 Count = ArrayCount(Array);

        VulkanReturnOnError(API->GetPhysicalDeviceSurfaceFormatsKHR(
            Vulkan->PhysicalDevice,
            Vulkan->Surface,
            &Count,
            Array
        ));

        Vulkan->SwapchainFormat.format = VK_FORMAT_UNDEFINED;

        for (usize Index = 0; Index < Count; Index++)
        {
            VkSurfaceFormatKHR Format = Array[Index];

            if (Format.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                Vulkan->SwapchainFormat = Format;
                break;
            }
            else if (Format.format == VK_FORMAT_R8G8B8A8_UNORM)
            {
                Vulkan->SwapchainFormat = Format;
                break;
            }
        }

        if (Vulkan->SwapchainFormat.format == VK_FORMAT_UNDEFINED)
            return (false);
    }

    // NOTE(vak): Swapchain present mode
    {
        VkPresentModeKHR Array[64] = {0};
        u32 Count = ArrayCount(Array);

        VulkanReturnOnError(API->GetPhysicalDeviceSurfacePresentModesKHR(
            Vulkan->PhysicalDevice,
            Vulkan->Surface,
            &Count,
            Array
        ));

        Vulkan->PresentMode = VK_PRESENT_MODE_FIFO_KHR;

        for (usize Index = 0; Index < Count; Index++)
        {
            VkPresentModeKHR Mode = Array[Index];

            if (Mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                Vulkan->PresentMode = Mode;
                break;
            }
        }
    }

    // NOTE(vak): Pipeline layout
    {
        VkPipelineLayoutCreateInfo LayoutInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };

        VulkanReturnOnError(API->CreatePipelineLayout(
            Vulkan->Device, &LayoutInfo, 0, &Vulkan->PipelineLayout
        ));
    }

    // NOTE(vak): Pipeline
    {
        persist u32 VertexCode[] =
        {
            #include "shaders/basic.vert.h"
        };

        persist u32 FragmentCode[] =
        {
            #include "shaders/basic.frag.h"
        };

        VkShaderModuleCreateInfo VertexModuleInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(VertexCode),
            .pCode = VertexCode,
        };

        VkShaderModuleCreateInfo FragmentModuleInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(FragmentCode),
            .pCode = FragmentCode,
        };

        VkShaderModule VertexModule = {0};
        VkShaderModule FragmentModule = {0};

        VulkanReturnOnError(API->CreateShaderModule(
            Vulkan->Device, &VertexModuleInfo, 0, &VertexModule
        ));

        VulkanReturnOnError(API->CreateShaderModule(
            Vulkan->Device, &FragmentModuleInfo, 0, &FragmentModule
        ));

        VkPipelineShaderStageCreateInfo Stages[2] =
        {
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = VertexModule,
                .pName = "main",
            },
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = FragmentModule,
                .pName = "main",
            },
        };

        VkPipelineVertexInputStateCreateInfo VertexInputState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };

        VkPipelineInputAssemblyStateCreateInfo InputAssemblyState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        };

        VkPipelineTessellationStateCreateInfo TessellationState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        };

        VkPipelineViewportStateCreateInfo ViewportState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &(VkViewport){0},
            .scissorCount = 1,
            .pScissors = &(VkRect2D){0},
        };

        VkPipelineRasterizationStateCreateInfo RasterizationState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo MultisampleState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };

        VkPipelineDepthStencilStateCreateInfo DepthStencilState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        };

        VkPipelineColorBlendStateCreateInfo ColorBlendState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &(VkPipelineColorBlendAttachmentState)
            {
                .blendEnable = VK_TRUE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT |
                    VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT,
            },
        };

        VkDynamicState DynamicStates[] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo DynamicState =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = ArrayCount(DynamicStates),
            .pDynamicStates = DynamicStates,
        };

        VkPipelineRenderingCreateInfo RenderingInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &Vulkan->SwapchainFormat.format,
        };

        VkGraphicsPipelineCreateInfo PipelineInfo =
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &RenderingInfo,
            .stageCount = ArrayCount(Stages),
            .pStages = Stages,
            .pVertexInputState = &VertexInputState,
            .pInputAssemblyState = &InputAssemblyState,
            .pTessellationState = &TessellationState,
            .pViewportState = &ViewportState,
            .pRasterizationState = &RasterizationState,
            .pMultisampleState = &MultisampleState,
            .pDepthStencilState = &DepthStencilState,
            .pColorBlendState = &ColorBlendState,
            .pDynamicState = &DynamicState,
            .layout = Vulkan->PipelineLayout,
        };

        VulkanReturnOnError(API->CreateGraphicsPipelines(
            Vulkan->Device,
            0,
            1,
            &PipelineInfo,
            0,
            &Vulkan->Pipeline
        ));

        API->DestroyShaderModule(Vulkan->Device, FragmentModule, 0);
        API->DestroyShaderModule(Vulkan->Device, VertexModule, 0);
    }

    #undef VulkanReturnOnError

    return (true);
}

local void VulkanRender(vulkan_state* Vulkan, vulkan_render_info* Info)
{
    #define VulkanReturnOnError(VulkanCall) \
        if ((VulkanCall) != VK_SUCCESS) return

    vulkan_api* API = &Vulkan->API;

    // NOTE(vak): Resize swapchain

    if ((Vulkan->SwapchainExtent.width != Info->TargetSizeX) ||
        (Vulkan->SwapchainExtent.height != Info->TargetSizeY))
    {
        VulkanReturnOnError(API->DeviceWaitIdle(Vulkan->Device));

        if (Vulkan->Swapchain)
        {
            for (usize Index = 0; Index < Vulkan->SwapchainImageCount; Index++)
                API->DestroyImageView(Vulkan->Device, Vulkan->SwapchainImageViews[Index], 0);

            API->DestroySwapchainKHR(Vulkan->Device, Vulkan->Swapchain, 0);
        }

        Vulkan->SwapchainExtent.width = Info->TargetSizeX;
        Vulkan->SwapchainExtent.height = Info->TargetSizeY;

        if ((Info->TargetSizeX == 0) || (Info->TargetSizeY == 0))
            return;

        VkSurfaceCapabilitiesKHR SurfaceCaps = {0};

        VulkanReturnOnError(API->GetPhysicalDeviceSurfaceCapabilitiesKHR(
            Vulkan->PhysicalDevice,
            Vulkan->Surface,
            &SurfaceCaps
        ));

        u32 MinImageCount = Clamp(SurfaceCaps.minImageCount, 3, SurfaceCaps.maxImageCount);

        VkSwapchainCreateInfoKHR SwapchainInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = Vulkan->Surface,
            .minImageCount = MinImageCount,
            .imageFormat = Vulkan->SwapchainFormat.format,
            .imageColorSpace = Vulkan->SwapchainFormat.colorSpace,
            .imageExtent = Vulkan->SwapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = SurfaceCaps.currentTransform,
            .presentMode = Vulkan->PresentMode,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .clipped = true,
        };

        VulkanReturnOnError(API->CreateSwapchainKHR(
            Vulkan->Device, &SwapchainInfo, 0, &Vulkan->Swapchain
        ));

        Vulkan->SwapchainImageCount = ArrayCount(Vulkan->SwapchainImages);

        VulkanReturnOnError(API->GetSwapchainImagesKHR(
            Vulkan->Device,
            Vulkan->Swapchain,
            &Vulkan->SwapchainImageCount,
            Vulkan->SwapchainImages
        ));

        for (u32 Index = 0; Index < Vulkan->SwapchainImageCount; Index++)
        {
            VkImageViewCreateInfo ViewInfo =
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = Vulkan->SwapchainImages[Index],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = Vulkan->SwapchainFormat.format,
                .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            };

            VulkanReturnOnError(API->CreateImageView(
                Vulkan->Device, &ViewInfo, 0, &Vulkan->SwapchainImageViews[Index]
            ));
        }

        VulkanReturnOnError(API->DeviceWaitIdle(Vulkan->Device));
    }

    u32 ImageIndex = 0;

    VulkanReturnOnError(API->AcquireNextImageKHR(
        Vulkan->Device,
        Vulkan->Swapchain,
        U64_MAX,
        Vulkan->AcquireSemaphore,
        0,
        &ImageIndex
    ));

    VulkanReturnOnError(API->ResetCommandBuffer(
        Vulkan->CommandBuffer,
        0
    ));

    VkCommandBufferBeginInfo BeginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VulkanReturnOnError(API->BeginCommandBuffer(
        Vulkan->CommandBuffer,
        &BeginInfo
    ));

    VkImageMemoryBarrier RenderBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_NONE,
        .dstAccessMask = VK_ACCESS_NONE,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = Vulkan->SwapchainImages[ImageIndex],
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };

    API->CmdPipelineBarrier(
        Vulkan->CommandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0, 0,
        0, 0,
        1, &RenderBarrier
    );

    VkRenderingInfo RenderingInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
        {
            .offset = {.x = 0, .y = 0},
            .extent = Vulkan->SwapchainExtent,
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &(VkRenderingAttachmentInfo)
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = Vulkan->SwapchainImageViews[ImageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue =
            {
                .color =
                {
                    .float32 = {0.07f, 0.08f, 0.1f, 1.0f},
                },
            },
        },
    };

    API->CmdBeginRendering(Vulkan->CommandBuffer, &RenderingInfo);

    API->CmdBindPipeline(Vulkan->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan->Pipeline);

    VkViewport Viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)Vulkan->SwapchainExtent.width,
        .height = (f32)Vulkan->SwapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D Scissor = RenderingInfo.renderArea;

    API->CmdSetViewport(Vulkan->CommandBuffer, 0, 1, &Viewport);
    API->CmdSetScissor(Vulkan->CommandBuffer, 0, 1, &Scissor);

    API->CmdDraw(Vulkan->CommandBuffer, 3, 1, 0, 0);

    API->CmdEndRendering(Vulkan->CommandBuffer);

    VkImageMemoryBarrier PresentBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_NONE,
        .dstAccessMask = VK_ACCESS_NONE,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = Vulkan->SwapchainImages[ImageIndex],
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };

    API->CmdPipelineBarrier(
        Vulkan->CommandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0, 0,
        0, 0,
        1, &PresentBarrier
    );

    VulkanReturnOnError(API->EndCommandBuffer(
        Vulkan->CommandBuffer
    ));

    VkPipelineStageFlags WaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo SubmitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Vulkan->AcquireSemaphore,
        .pWaitDstStageMask = &WaitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &Vulkan->CommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &Vulkan->SubmitSemaphore,
    };

    VulkanReturnOnError(API->QueueSubmit(
        Vulkan->Queue, 1, &SubmitInfo, 0
    ));

    VkPresentInfoKHR PresentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Vulkan->SubmitSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &Vulkan->Swapchain,
        .pImageIndices = &ImageIndex,
    };

    VulkanReturnOnError(API->QueuePresentKHR(
        Vulkan->Queue, &PresentInfo
    ));

    VulkanReturnOnError(API->DeviceWaitIdle(Vulkan->Device));

    #undef VulkanReturnOnError
}

