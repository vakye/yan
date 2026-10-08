
#pragma once

typedef struct
{
    VkExtent2D      Extent;
    VkFormat        Format;
    VkSwapchainKHR  Swapchain;
    u32             ImageCount;
    VkImage         Images[16];
    VkImageView     ImageViews[16];
} vulkan_swapchain;

local void VulkanResizeSwapchain(
    vulkan_swapchain* Swapchain,
    vulkan_api* API,
    vulkan_device* Device,
    VkSurfaceKHR Surface,
    VkPresentModeKHR PresentMode,
    VkSurfaceFormatKHR Format,
    u32 Width,
    u32 Height
)
{
    if ((Swapchain->Extent.width == Width) &&
        (Swapchain->Extent.height == Height))
    {
        return;
    }

    VulkanCheck(API->DeviceWaitIdle(Device->Device));

    if (Swapchain->Swapchain)
    {
        for (usize Index = 0; Index < Swapchain->ImageCount; Index++)
            API->DestroyImageView(Device->Device, Swapchain->ImageViews[Index], 0);

        API->DestroySwapchainKHR(Device->Device, Swapchain->Swapchain, 0);
    }

    Swapchain->Extent.width = Width;
    Swapchain->Extent.height = Height;
    Swapchain->Format = Format.format;

    if (Swapchain->Extent.width == 0) return;
    if (Swapchain->Extent.height == 0) return;

    VkSurfaceCapabilitiesKHR SurfaceCaps = {0};

    VulkanCheck(API->GetPhysicalDeviceSurfaceCapabilitiesKHR(
        Device->Physical,
        Surface,
        &SurfaceCaps
    ));

    u32 MinImageCount = Clamp(SurfaceCaps.minImageCount, 3, SurfaceCaps.maxImageCount);

    VkSwapchainCreateInfoKHR SwapchainInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = Surface,
        .minImageCount = MinImageCount,
        .imageFormat = Format.format,
        .imageColorSpace = Format.colorSpace,
        .imageExtent = {.width = Width, .height = Height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = SurfaceCaps.currentTransform,
        .presentMode = PresentMode,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .clipped = true,
    };

    VulkanCheck(API->CreateSwapchainKHR(
        Device->Device, &SwapchainInfo, 0, &Swapchain->Swapchain
    ));

    Swapchain->ImageCount = ArrayCount(Swapchain->Images);

    VulkanCheck(API->GetSwapchainImagesKHR(
        Device->Device,
        Swapchain->Swapchain,
        &Swapchain->ImageCount,
        Swapchain->Images
    ));

    for (u32 Index = 0; Index < Swapchain->ImageCount; Index++)
    {
        VkImageViewCreateInfo ViewInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = Swapchain->Images[Index],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = Format.format,
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

        VulkanCheck(API->CreateImageView(
            Device->Device, &ViewInfo, 0, &Swapchain->ImageViews[Index]
        ));
    }

    VulkanCheck(API->DeviceWaitIdle(Device->Device));
}

