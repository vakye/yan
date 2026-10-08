
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>

#define VulkanCheck(VulkanCall) \
    if ((VulkanCall) != VK_SUCCESS) Assert(false)

#include "vulkan_api.c"
#include "vulkan_device.c"
#include "vulkan_resources.c"
#include "vulkan_swapchain.c"

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
    u32             TargetSizeX;
    u32             TargetSizeY;
    render_array*   Renders;
} vulkan_render_info;

typedef struct
{
    u32                     VersionOfAPI;
    VkInstance              Instance;
    VkSurfaceKHR            Surface;
    vulkan_device           Device;
    VkCommandPool           CommandPool;
    VkCommandBuffer         CommandBuffer;
    VkSemaphore             AcquireSemaphore;
    VkSemaphore             SubmitSemaphore;
    VkSurfaceFormatKHR      SwapchainFormat;
    VkPresentModeKHR        PresentMode;
    VkDescriptorSetLayout   SetLayout;
    VkPipelineLayout        PipelineLayout;
    VkPipeline              Pipeline;
    vulkan_buffer           VertexBuffer;
    vulkan_swapchain        Swapchain;
    vulkan_api              API;
} vulkan_state;

local b32   VulkanSetup     (vulkan_state* Vulkan, vulkan_setup_info* Info);
local void  VulkanRender    (vulkan_state* Vulkan, vulkan_render_info* Info);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================

typedef struct
{
    f32 X, Y;
    f32 U, V;
    f32 R, G, B, A;
} vulkan_vertex;

typedef struct
{
    f32 Projection[16];
} vulkan_push_constants;

local b32 VulkanSetup(vulkan_state* Vulkan, vulkan_setup_info* Info)
{
    if (!Info->vkGetInstanceProcAddr) return (false);

    vulkan_api* API = &Vulkan->API;

    if (!VulkanLoadNonInstanceAPI(API, Info->vkGetInstanceProcAddr))
        return (false);

    // NOTE(vak): Create instance
    {
        Vulkan->VersionOfAPI = VK_API_VERSION_1_4;

        u32 InstanceVersion = 0;
        VulkanCheck(
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

        VulkanCheck(API->CreateInstance(
            &InstanceInfo, 0, &Vulkan->Instance
        ));
    }

    b32 LoadWayland = (Info->SurfaceKind == VulkanSurfaceKind_Wayland);

    if (!VulkanLoadInstanceAPI(API, Vulkan->Instance, LoadWayland))
        return (false);

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

                VulkanCheck(API->CreateWaylandSurfaceKHR(
                    Vulkan->Instance, &SurfaceInfo, 0, &Vulkan->Surface
                ));
            } break;
        }
    }

    vulkan_device* Device = &Vulkan->Device;

    if (!VulkanCreateDevice(Device, API, Vulkan->Instance, Vulkan->VersionOfAPI))
        return (false);

    // NOTE(vak): Command pool
    {
        VkCommandPoolCreateInfo PoolInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = Device->QueueFamilyIndex,
        };

        VulkanCheck(API->CreateCommandPool(
            Device->Device, &PoolInfo, 0, &Vulkan->CommandPool
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

        VulkanCheck(API->AllocateCommandBuffers(
            Device->Device, &AllocateInfo, &Vulkan->CommandBuffer
        ));
    }

    // NOTE(vak): Semaphores
    {
        VkSemaphoreCreateInfo SemaphoreInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VulkanCheck(API->CreateSemaphore(
            Device->Device, &SemaphoreInfo, 0, &Vulkan->AcquireSemaphore
        ));

        VulkanCheck(API->CreateSemaphore(
            Device->Device, &SemaphoreInfo, 0, &Vulkan->SubmitSemaphore
        ));
    }

    // NOTE(vak): Swapchain format
    {
        VkSurfaceFormatKHR Array[512] = {0};
        u32 Count = ArrayCount(Array);

        VulkanCheck(API->GetPhysicalDeviceSurfaceFormatsKHR(
            Device->Physical,
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

        VulkanCheck(API->GetPhysicalDeviceSurfacePresentModesKHR(
            Device->Physical,
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

    // NOTE(vak): Descriptor set layout
    {
        VkDescriptorSetLayoutBinding Bindings[] =
        {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            },
        };

        VkDescriptorSetLayoutCreateInfo SetLayoutInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
            .bindingCount = ArrayCount(Bindings),
            .pBindings = Bindings,
        };

        VulkanCheck(API->CreateDescriptorSetLayout(
            Device->Device,
            &SetLayoutInfo,
            0,
            &Vulkan->SetLayout
        ));
    }

    // NOTE(vak): Pipeline layout
    {
        VkPipelineLayoutCreateInfo LayoutInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &Vulkan->SetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &(VkPushConstantRange)
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(vulkan_push_constants), 
            },
        };

        VulkanCheck(API->CreatePipelineLayout(
            Device->Device, &LayoutInfo, 0, &Vulkan->PipelineLayout
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

        VulkanCheck(API->CreateShaderModule(
            Device->Device, &VertexModuleInfo, 0, &VertexModule
        ));

        VulkanCheck(API->CreateShaderModule(
            Device->Device, &FragmentModuleInfo, 0, &FragmentModule
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

        VulkanCheck(API->CreateGraphicsPipelines(
            Device->Device,
            0,
            1,
            &PipelineInfo,
            0,
            &Vulkan->Pipeline
        ));

        API->DestroyShaderModule(Device->Device, FragmentModule, 0);
        API->DestroyShaderModule(Device->Device, VertexModule, 0);
    }

    // NOTE(vak): Buffers
    {
        u32 MaxRectPerDraw = 4096;
        u32 MaxVerticesPerDraw = MaxRectPerDraw * 6;
        u32 VertexBufferSize = MaxVerticesPerDraw * sizeof(vulkan_vertex);

        if (!VulkanCreateBuffer(
            &Vulkan->VertexBuffer,
            API,
            Device,
            VertexBufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            true
        ))
        {
            return (false);
        }
    }

    return (true);
}

local void VulkanRender(vulkan_state* Vulkan, vulkan_render_info* Info)
{
    vulkan_api* API = &Vulkan->API;
    vulkan_device* Device = &Vulkan->Device;
    vulkan_swapchain* Swapchain = &Vulkan->Swapchain;

    // NOTE(vak): Resize swapchain

    VulkanResizeSwapchain(
        Swapchain,
        API,
        Device,
        Vulkan->Surface,
        Vulkan->PresentMode,
        Vulkan->SwapchainFormat,
        Info->TargetSizeX,
        Info->TargetSizeY
    );

    if (Swapchain->Extent.width == 0) return;
    if (Swapchain->Extent.height == 0) return;

    u32 VertexCount = 0;

    if (Info->Renders)
    {
        render_array* Renders = Info->Renders;

        u32 MaxVertices = Vulkan->VertexBuffer.Size / sizeof(vulkan_vertex);
        u32 VerticesNeeded = Renders->RectCount;

        Assert(MaxVertices >= VerticesNeeded);

        vulkan_vertex* Vertices = (vulkan_vertex*)Vulkan->VertexBuffer.Mapping;

        for (usize Index = 0; Index < Renders->RectCount; Index++)
        {
            render_rect* Rect = Renders->Rects + Index;

            f32 MinX = Rect->X;
            f32 MinY = Rect->Y;
            f32 MaxX = Rect->X + Rect->W;
            f32 MaxY = Rect->Y + Rect->H;

            f32 R = Rect->R;
            f32 G = Rect->G;
            f32 B = Rect->B;
            f32 A = Rect->A;

            vulkan_vertex* V = Vertices + VertexCount;

            V[0] = (vulkan_vertex){MinX, MinY, 0.0f, 0.0f, R, G, B, A};
            V[1] = (vulkan_vertex){MinX, MaxY, 0.0f, 1.0f, R, G, B, A};
            V[2] = (vulkan_vertex){MaxX, MaxY, 1.0f, 1.0f, R, G, B, A};

            V[3] = (vulkan_vertex){MaxX, MaxY, 1.0f, 1.0f, R, G, B, A};
            V[4] = (vulkan_vertex){MaxX, MinY, 1.0f, 0.0f, R, G, B, A};
            V[5] = (vulkan_vertex){MinX, MinY, 0.0f, 0.0f, R, G, B, A};

            VertexCount += 6;
        }
    }

    u32 ImageIndex = 0;

    VulkanCheck(API->AcquireNextImageKHR(
        Device->Device,
        Swapchain->Swapchain,
        U64_MAX,
        Vulkan->AcquireSemaphore,
        0,
        &ImageIndex
    ));

    VulkanCheck(API->ResetCommandBuffer(
        Vulkan->CommandBuffer,
        0
    ));

    VkCommandBufferBeginInfo BeginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VulkanCheck(API->BeginCommandBuffer(
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
        .image = Swapchain->Images[ImageIndex],
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
            .extent = Swapchain->Extent,
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &(VkRenderingAttachmentInfo)
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = Swapchain->ImageViews[ImageIndex],
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

    if (VertexCount)
    {
        API->CmdBindPipeline(Vulkan->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan->Pipeline);

        VkViewport Viewport =
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = (f32)Swapchain->Extent.width,
            .height = (f32)Swapchain->Extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D Scissor = RenderingInfo.renderArea;

        API->CmdSetViewport(Vulkan->CommandBuffer, 0, 1, &Viewport);
        API->CmdSetScissor(Vulkan->CommandBuffer, 0, 1, &Scissor);

        VkWriteDescriptorSet DescriptorWrites[] =
        {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &(VkDescriptorBufferInfo)
                {
                    .buffer = Vulkan->VertexBuffer.Buffer,
                    .offset = 0,
                    .range = Vulkan->VertexBuffer.Size,
                },
            },
        };

        API->CmdPushDescriptorSet(
            Vulkan->CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            Vulkan->PipelineLayout,
            0,
            ArrayCount(DescriptorWrites),
            DescriptorWrites
        );

        vulkan_push_constants PushConstants =
        {
            .Projection =
            {
                [0]     = 2.0f / (f32)Swapchain->Extent.width,
                [5]     = 2.0f / (f32)Swapchain->Extent.height,
                [10]    = 1.0f,
                [15]    = 1.0f,

                [12]    = -1.0f,
                [13]    = -1.0f,
            },
        };

        API->CmdPushConstants(
            Vulkan->CommandBuffer,
            Vulkan->PipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PushConstants),
            &PushConstants
        );

        API->CmdDraw(Vulkan->CommandBuffer, VertexCount, 1, 0, 0);
    }

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
        .image = Swapchain->Images[ImageIndex],
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

    VulkanCheck(API->EndCommandBuffer(
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

    VulkanCheck(API->QueueSubmit(
        Device->Queue, 1, &SubmitInfo, 0
    ));

    VkPresentInfoKHR PresentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &Vulkan->SubmitSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &Swapchain->Swapchain,
        .pImageIndices = &ImageIndex,
    };

    VulkanCheck(API->QueuePresentKHR(
        Device->Queue, &PresentInfo
    ));

    VulkanCheck(API->DeviceWaitIdle(Device->Device));
}

