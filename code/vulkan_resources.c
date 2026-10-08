
#pragma once

typedef struct
{
    VkBuffer        Buffer;
    VkDeviceMemory  Memory;
    usize           Size;
    void*           Mapping;
} vulkan_buffer;

local u32 VulkanSelectMemoryType(
    vulkan_api* API,
    vulkan_device* Device,
    VkMemoryPropertyFlags RequiredFlags,
    VkMemoryRequirements* Requirements
)
{
    VkPhysicalDeviceMemoryProperties Properties = {0};
    API->GetPhysicalDeviceMemoryProperties(Device->Physical, &Properties);

    u32 TypeBits = Requirements->memoryTypeBits;
    u32 Result = U32_MAX;

    for (u32 Index = 0; Index < Properties.memoryTypeCount; Index++)
    {
        VkMemoryType* MemoryType = Properties.memoryTypes + Index;

        if ((TypeBits & (1 << Index)) == 0)
            continue;

        if ((MemoryType->propertyFlags & RequiredFlags) != RequiredFlags)
            continue;

        Result = Index;
        break;
    }

    return (Result);
}

local b32 VulkanCreateBuffer(
    vulkan_buffer* Buffer,
    vulkan_api* API,
    vulkan_device* Device,
    usize Size,
    VkBufferUsageFlags UsageFlags,
    VkMemoryPropertyFlags MemoryPropertyFlags,
    b32 Mapped
)
{
    #define VulkanReturnOnError(VulkanCall) if ((VulkanCall) != VK_SUCCESS) return (false)

    Buffer->Size = Size;

    VkBufferCreateInfo BufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = Size,
        .usage = UsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VulkanReturnOnError(API->CreateBuffer(
        Device->Device, &BufferInfo, 0, &Buffer->Buffer
    ));

    VkMemoryRequirements MemoryRequirements = {0};
    API->GetBufferMemoryRequirements(
        Device->Device,
        Buffer->Buffer,
        &MemoryRequirements
    );

    VkMemoryAllocateInfo AllocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = MemoryRequirements.size,
        .memoryTypeIndex = VulkanSelectMemoryType(
            API, Device, MemoryPropertyFlags, &MemoryRequirements
        ),
    };

    VulkanReturnOnError(API->AllocateMemory(
        Device->Device,
        &AllocateInfo,
        0,
        &Buffer->Memory
    ));

    VulkanReturnOnError(API->BindBufferMemory(
        Device->Device,
        Buffer->Buffer,
        Buffer->Memory,
        0
    ));

    if (Mapped)
    {
        VulkanReturnOnError(API->MapMemory(
            Device->Device,
            Buffer->Memory,
            0,
            Buffer->Size,
            0,
            &Buffer->Mapping
        ));
    }

    #undef VulkanReturnOnError

    return (true);
}

