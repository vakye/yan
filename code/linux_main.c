
#include "shared.c"
#include "render.c"
#include "platform.c"
#include "game.c"

#include "vulkan_render.c"
#include "wayland_window.c"
#include "linux_platform.c"

local void* LinuxLoadVkGetInstanceProcAddr(void)
{
    void* Library = dlopen("libvulkan.so", RTLD_NOW|RTLD_LOCAL);

    if (!Library)
        Library = dlopen("libvulkan.so.1", RTLD_NOW|RTLD_LOCAL);

    void* Result = 0;

    if (Library)
        Result = dlsym(Library, "vkGetInstanceProcAddr");

    return (Result);
}

void LinuxEntry(s32 ArgCount, char* Args[], char* Envp[])
{
    wayland_state Wayland = {0};
    b32 WaylandGood = WaylandSetup(&Wayland);

    ExitErrorIfNot(WaylandGood);

    vulkan_state Vulkan = {0};
    b32 VulkanGood = VulkanSetup(&Vulkan, &(vulkan_setup_info)
    {
        .vkGetInstanceProcAddr = LinuxLoadVkGetInstanceProcAddr(),
        .SurfaceKind = VulkanSurfaceKind_Wayland,
        .WaylandDisplay = WaylandGetDisplay(&Wayland),
        .WaylandSurface = WaylandGetSurface(&Wayland),
    });

    ExitErrorIfNot(VulkanGood);

    render_array Renders = {0};
    {
        persist render_rect Memory[4096] = {0};

        Renders.Rects           = Memory;
        Renders.MaxRectCount    = ArrayCount(Memory);
    }

    while (!WaylandIsClosed(&Wayland))
    {
        WaylandPollEvents(&Wayland);

        GameRender(&Renders);

        VulkanRender(&Vulkan, &(vulkan_render_info)
        {
            .TargetSizeX = WaylandGetSizeX(&Wayland),
            .TargetSizeY = WaylandGetSizeY(&Wayland),
            .Renders     = &Renders,
        });

        WaylandPresent(&Wayland);
    }

    Exit(0);
}

