
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

#include <wayland-client.h>
#include "wayland_xdg.c"

typedef struct
{
    struct wl_display*      Display;
    struct wl_registry*     Registry;
    struct wl_compositor*   Compositor;
    struct xdg_wm_base*     XdgWmBase;
    struct wl_surface*      Surface;
    struct xdg_surface*     XdgSurface;
    struct xdg_toplevel*    XdgTopLevel;

    u32 TopLevelSizeX, TopLevelSizeY;
    u32 SizeX, SizeY;
    b32 IsClosed;
} wayland_state;

local b32                   WaylandSetup        (wayland_state* Wayland);
local struct wl_display*    WaylandGetDisplay   (wayland_state* Wayland);
local struct wl_surface*    WaylandGetSurface   (wayland_state* Wayland);
local b32                   WaylandIsClosed     (wayland_state* Wayland);
local u32                   WaylandGetSizeX     (wayland_state* Wayland);
local u32                   WaylandGetSizeY     (wayland_state* Wayland);
local void                  WaylandPollEvents   (wayland_state* Wayland);
local void                  WaylandPresent      (wayland_state* Wayland);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================

local void WaylandRegistryGlobalEvent(
    void*               Data,
    struct wl_registry* Registry,
    u32                 Name,
    const char*         Interface,
    u32                 Version
)
{
    wayland_state* Wayland = (wayland_state*)Data;
    string InterfaceString = CString(Interface);

    if (StringEquals(InterfaceString, CString(wl_compositor_interface.name)))
    {
        Wayland->Compositor = wl_registry_bind(Registry, Name, &wl_compositor_interface, Version);
    }
    else if (StringEquals(InterfaceString, CString(xdg_wm_base_interface.name)))
    {
        Wayland->XdgWmBase = wl_registry_bind(Registry, Name, &xdg_wm_base_interface, Version);
    }
}

local void WaylandRegistryGlobalRemoveEvent(
    void*               Data,
    struct wl_registry* Registry,
    u32                 Name
)
{
}

local struct wl_registry_listener WaylandRegistryListener =
{
    .global = &WaylandRegistryGlobalEvent,
    .global_remove = &WaylandRegistryGlobalRemoveEvent,
};

local void WaylandXdgWmBasePingEvent(
    void*               Data,
    struct xdg_wm_base* XdgWmBase,
    u32                 Serial
)
{
    xdg_wm_base_pong(XdgWmBase, Serial);
}

local struct xdg_wm_base_listener WaylandXdgWmBaseListener =
{
    .ping = WaylandXdgWmBasePingEvent,
};

local void WaylandXdgSurfaceConfigureEvent(
    void*               Data,
    struct xdg_surface* XdgSurface,
    u32                 Serial
)
{
    wayland_state* Wayland = (wayland_state*)Data;

    xdg_surface_ack_configure(XdgSurface, Serial);

    Wayland->SizeX = Wayland->TopLevelSizeX;
    Wayland->SizeY = Wayland->TopLevelSizeY;
}

local struct xdg_surface_listener WaylandXdgSurfaceListener =
{
    .configure = &WaylandXdgSurfaceConfigureEvent,
};

local void WaylandXdgTopLevelConfigureEvent(
    void*                   Data,
    struct xdg_toplevel*    XdgTopLevel,
    s32                     Width,
    s32                     Height,
    struct wl_array*        States
)
{
    wayland_state* Wayland = (wayland_state*)Data;

    Wayland->TopLevelSizeX = Maximum(0, Width);
    Wayland->TopLevelSizeY = Maximum(0, Height);
}

local void WaylandXdgTopLevelCloseEvent(
    void*                   Data,
    struct xdg_toplevel*    XdgTopLevel
)
{
    wayland_state* Wayland = (wayland_state*)Data;
    Wayland->IsClosed = true;
} 

local void WaylandXdgTopLevelConfigureBoundsEvent(
    void*                   Data,
    struct xdg_toplevel*    XdgTopLevel,
    s32                     Width,
    s32                     Height
)
{
}

local void WaylandXdgTopLevelWmCapabilitiesEvent(
    void*                   Data,
    struct xdg_toplevel*    XdgTopLevel,
    struct wl_array*        Capabilities
)
{
}

local struct xdg_toplevel_listener WaylandXdgTopLevelListener =
{
    .configure = &WaylandXdgTopLevelConfigureEvent,
    .close = &WaylandXdgTopLevelCloseEvent,
    .configure_bounds = &WaylandXdgTopLevelConfigureBoundsEvent,
    .wm_capabilities = &WaylandXdgTopLevelWmCapabilitiesEvent,
};

local b32 WaylandSetup(wayland_state* Wayland)
{
    if (!Wayland) return (false);

    Wayland->Display = wl_display_connect(0);
    if (!Wayland->Display) return (false);

    Wayland->Registry = wl_display_get_registry(Wayland->Display);
    if (!Wayland->Registry) return (false);

    wl_registry_add_listener(Wayland->Registry, &WaylandRegistryListener, Wayland);
    wl_display_roundtrip(Wayland->Display);

    if (!Wayland->Compositor) return (false);
    if (!Wayland->XdgWmBase) return (false);

    Wayland->Surface = wl_compositor_create_surface(Wayland->Compositor);
    if (!Wayland->Surface) return (false);

    Wayland->XdgSurface = xdg_wm_base_get_xdg_surface(Wayland->XdgWmBase, Wayland->Surface);
    if (!Wayland->XdgSurface) return (false);

    Wayland->XdgTopLevel = xdg_surface_get_toplevel(Wayland->XdgSurface);
    if (!Wayland->XdgTopLevel) return (false);

    xdg_toplevel_set_title(Wayland->XdgTopLevel, "yan");
    xdg_toplevel_set_app_id(Wayland->XdgTopLevel, "yan");

    xdg_wm_base_add_listener(Wayland->XdgWmBase, &WaylandXdgWmBaseListener, Wayland);
    xdg_surface_add_listener(Wayland->XdgSurface, &WaylandXdgSurfaceListener, Wayland);
    xdg_toplevel_add_listener(Wayland->XdgTopLevel, &WaylandXdgTopLevelListener, Wayland);

    wl_surface_commit(Wayland->Surface);
    wl_display_roundtrip(Wayland->Display);
    wl_surface_commit(Wayland->Surface);

    return (true);
}

local struct wl_display* WaylandGetDisplay(wayland_state* Wayland)
{
    return (Wayland->Display);
}

local struct wl_surface* WaylandGetSurface(wayland_state* Wayland)
{
    return (Wayland->Surface);
}

local b32 WaylandIsClosed(wayland_state* Wayland)
{
    return (Wayland->IsClosed);
}

local u32 WaylandGetSizeX(wayland_state* Wayland)
{
    return (Wayland->SizeX);
}

local u32 WaylandGetSizeY(wayland_state* Wayland)
{
    return (Wayland->SizeY);
}

local void WaylandPollEvents(wayland_state* Wayland)
{
    wl_display_roundtrip(Wayland->Display);
}

local void WaylandPresent(wayland_state* Wayland)
{
    wl_surface_commit(Wayland->Surface);
}

