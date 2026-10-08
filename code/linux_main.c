
#include "shared.c"
#include "render.c"
#include "input.c"
#include "platform.c"
#include "game.c"

#include "vulkan_render.c"
#include "linux_platform.c"
#include "wayland_window.c"

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

    input_array Inputs = {0};
    {
        persist input_event Memory[1024] = {0};

        Inputs.Events = Memory;
        Inputs.MaxEventCount = ArrayCount(Memory);
    }

    render_array Renders = {0};
    {
        persist render_rect Memory[4096] = {0};

        Renders.Rects = Memory;
        Renders.MaxRectCount = ArrayCount(Memory);
    }

    game_state Game = {0};
    {
        GameSetup(&Game);
    }

    f32 UpdateTimeStep  = 1.0f/80.0f;
    f32 RenderTimeStep  = 1.0f/WaylandGetRefreshRate(&Wayland);
    f32 UpdateTimer     = 0.0f;

    usize FrameBegin = GetWallClock();

    while (!WaylandIsClosed(&Wayland))
    {
        WaylandPollEvents(&Wayland, &Inputs);

        while (UpdateTimer >= UpdateTimeStep)
        {
            GameUpdate(&Game, &Inputs, UpdateTimeStep);
            ResetInputs(&Inputs);

            UpdateTimer -= UpdateTimeStep;
        }

        ResetRenders(&Renders);
        GameRender(&Game, &Renders, UpdateTimer);

        VulkanRender(&Vulkan, &(vulkan_render_info)
        {
            .TargetSizeX = WaylandGetSizeX(&Wayland),
            .TargetSizeY = WaylandGetSizeY(&Wayland),
            .Renders     = &Renders,
        });

        WaylandPresent(&Wayland);

        f64 Elapsed = GetSecondsElapsed(FrameBegin, GetWallClock());

        if (Elapsed < RenderTimeStep)
            Wait(RenderTimeStep - Elapsed);

        UpdateTimer += Elapsed;
        FrameBegin = GetWallClock();
    }

    Exit(0);
}

