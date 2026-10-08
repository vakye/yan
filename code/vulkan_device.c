
#pragma once

typedef struct
{
    VkPhysicalDevice    Physical;
    u32                 QueueFamilyIndex;
    VkDevice            Device;
    VkQueue             Queue;
} vulkan_device;

local b32 VulkanCreateDevice(
    vulkan_device* Device,
    vulkan_api* API,
    VkInstance Instance,
    u32 VersionOfAPI
)
{
    // NOTE(vak): Physical device
    {
        VkPhysicalDevice Array[64] = {0};
        u32 Count = ArrayCount(Array);

        VulkanCheck(API->EnumeratePhysicalDevices(
            Instance,
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

            if (Properties.apiVersion < VersionOfAPI)
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

        Device->Physical = (Preferred) ? (Preferred) : (Fallback);
        if (!Device->Physical) return (false);
    }

    // NOTE(vak): Queue family
    {
        Device->QueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VkQueueFamilyProperties Array[64] = {0};
        u32 Count = ArrayCount(Array);

        API->GetPhysicalDeviceQueueFamilyProperties(
            Device->Physical,
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
                Device->QueueFamilyIndex = Index;
                break;
            }
        }

        if (Device->QueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
            return (false);
    }

    // NOTE(vak): Device
    {
        const char* Extensions[] =
        {
            "VK_KHR_swapchain",
        };

        VkPhysicalDeviceVulkan14Features Vulkan14Features =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            .pushDescriptor = true,
        };

        VkPhysicalDeviceVulkan13Features Vulkan13Features =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &Vulkan14Features,
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
                .queueFamilyIndex = Device->QueueFamilyIndex,
                .queueCount = 1,
                .pQueuePriorities = (f32[1]){1.0f},
            },
            .enabledExtensionCount = ArrayCount(Extensions),
            .ppEnabledExtensionNames = Extensions,
        };

        VulkanCheck(API->CreateDevice(
            Device->Physical, &DeviceInfo, 0, &Device->Device
        ));
    }

    // NOTE(vak): Queue
    {
        API->GetDeviceQueue(
            Device->Device, Device->QueueFamilyIndex, 0, &Device->Queue
        );
    }

    return (true);
}

