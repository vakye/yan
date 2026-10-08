
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

#include <xkbcommon/xkbcommon.h>
#include <wayland-client.h>
#include "wayland_xdg.c"

typedef struct
{
    struct wl_display*      Display;
    struct wl_registry*     Registry;
    struct wl_output*       Output;
    struct wl_compositor*   Compositor;
    struct xdg_wm_base*     XdgWmBase;
    struct wl_surface*      Surface;
    struct xdg_surface*     XdgSurface;
    struct xdg_toplevel*    XdgTopLevel;

    struct wl_seat*         Seat;
    struct wl_keyboard*     Keyboard;

    struct xkb_context*     XkbContext;
    struct xkb_keymap*      XkbKeymap;
    struct xkb_state*       XkbState;

    input_array*            Inputs;

    u32 TopLevelSizeX, TopLevelSizeY;
    u32 SizeX, SizeY;
    b32 IsClosed;
    f32 RefreshRate;
} wayland_state;

local b32                   WaylandSetup            (wayland_state* Wayland);
local struct wl_display*    WaylandGetDisplay       (wayland_state* Wayland);
local struct wl_surface*    WaylandGetSurface       (wayland_state* Wayland);
local b32                   WaylandIsClosed         (wayland_state* Wayland);
local u32                   WaylandGetSizeX         (wayland_state* Wayland);
local u32                   WaylandGetSizeY         (wayland_state* Wayland);
local f32                   WaylandGetRefreshRate   (wayland_state* Wayland);
local void                  WaylandPollEvents       (wayland_state* Wayland, input_array* Inputs);
local void                  WaylandPresent          (wayland_state* Wayland);

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
    else if (StringEquals(InterfaceString, CString(wl_output_interface.name)))
    {
        Wayland->Output = wl_registry_bind(Registry, Name, &wl_output_interface, Version);
    }
    else if (StringEquals(InterfaceString, CString(wl_seat_interface.name)))
    {
        Wayland->Seat = wl_registry_bind(Registry, Name, &wl_seat_interface, Version);
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

local void WaylandOutputGeometryEvent(
    void*               Data,
    struct wl_output*   Output,
    s32                 X,
    s32                 Y,
    s32                 PhysicalWidth,
    s32                 PhysicalHeight,
    s32                 Subpixel,
    const char*         Make,
    const char*         Model,
    s32                 Transform
)
{
}

local void WaylandOutputModeEvent(
    void*               Data,
    struct wl_output*   Output,
    u32                 Flags,
    s32                 Width,
    s32                 Height,
    s32                 Refresh
)
{
    wayland_state* Wayland = (wayland_state*)Data;

    if (Flags & WL_OUTPUT_MODE_CURRENT)
    {
        Wayland->RefreshRate = Maximum(0, Refresh) / 1000.0f;
    }
}

local void WaylandOutputDoneEvent(
    void*               Data,
    struct wl_output*   Output
)
{
}

local void WaylandOutputScaleEvent(
    void*               Data,
    struct wl_output*   Output,
    s32                 Factor
)
{
}

local void WaylandOutputNameEvent(
    void*               Data,
    struct wl_output*   Output,
    const char*         Name
)
{
}

local void WaylandOutputDescriptionEvent(
    void*               Data,
    struct wl_output*   Output,
    const char*         Description
)
{
}

local struct wl_output_listener WaylandOutputListener =
{
    .geometry = &WaylandOutputGeometryEvent,
    .mode = &WaylandOutputModeEvent,
    .done = &WaylandOutputDoneEvent,
    .scale = &WaylandOutputScaleEvent,
    .name = &WaylandOutputNameEvent,
    .description = &WaylandOutputDescriptionEvent,
};

local void WaylandKeyboardKeymapEvent(
    void*               Data,
    struct wl_keyboard* Keyboard,
    u32                 Format,
    s32                 FileDescriptor,
    u32                 Size
)
{
    wayland_state* Wayland = (wayland_state*)Data;

    if (Format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
        return;

    void* KeymapString = mmap(0, Size, PROT_READ, MAP_PRIVATE, FileDescriptor, 0);
    if (!KeymapString)
        return;

    Wayland->XkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!Wayland->XkbContext)
        return;

    Wayland->XkbKeymap = xkb_keymap_new_from_string(Wayland->XkbContext, KeymapString, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!Wayland->XkbKeymap)
        return;

    Wayland->XkbState = xkb_state_new(Wayland->XkbKeymap);
}

local void WaylandKeyboardEnterEvent(
    void*               Data,
    struct wl_keyboard* Keyboard,
    u32                 Serial,
    struct wl_surface*  Surface,
    struct wl_array*    Keys
)
{
}

local void WaylandKeyboardLeaveEvent(
    void*               Data,
    struct wl_keyboard* Keyboard,
    u32                 Serial,
    struct wl_surface*  Surface
)
{
}

local void WaylandKeyboardKeyEvent(
    void*               Data,
    struct wl_keyboard* Keyboard,
    u32                 Serial,
    u32                 Time,
    u32                 EvdevScancode,
    u32                 State
)
{
    wayland_state* Wayland = (wayland_state*)Data;

    if (!Wayland->XkbState)
        return;

    u32 XkbScancode = EvdevScancode + 8;
    xkb_keysym_t KeySym = xkb_state_key_get_one_sym(Wayland->XkbState, XkbScancode);

    b32 IsDown = (State == WL_KEYBOARD_KEY_STATE_PRESSED);

    input_button Button = InputButton_Nil;

    switch (KeySym)
    {
        default: break;

        case XKB_KEY_w: case XKB_KEY_W: Button = InputButton_KeyW; break;
        case XKB_KEY_a: case XKB_KEY_A: Button = InputButton_KeyA; break;
        case XKB_KEY_s: case XKB_KEY_S: Button = InputButton_KeyS; break;
        case XKB_KEY_d: case XKB_KEY_D: Button = InputButton_KeyD; break;

        case XKB_KEY_Up:    Button = InputButton_KeyUp;     break;
        case XKB_KEY_Down:  Button = InputButton_KeyDown;   break;
        case XKB_KEY_Left:  Button = InputButton_KeyLeft;   break;
        case XKB_KEY_Right: Button = InputButton_KeyRight;  break;
    }

    AddInputEventButton(Wayland->Inputs, Button, IsDown);
}

local void WaylandKeyboardModifiersEvent(
    void*               Data,
    struct wl_keyboard* Keyboard,
    u32                 Serial,
    u32                 ModifiersDepressed,
    u32                 ModifiersLatched,
    u32                 ModifiersLocked,
    u32                 Group
)
{
}

local void WaylandKeyboardRepeatInfoEvent(
    void*               Data,
    struct wl_keyboard* Keyboard,
    s32                 Rate,
    s32                 Delay
)
{
}

local struct wl_keyboard_listener WaylandKeyboardListener =
{
    .keymap = &WaylandKeyboardKeymapEvent,
    .enter = &WaylandKeyboardEnterEvent,
    .leave = &WaylandKeyboardLeaveEvent,
    .key = &WaylandKeyboardKeyEvent,
    .modifiers = &WaylandKeyboardModifiersEvent,
    .repeat_info = &WaylandKeyboardRepeatInfoEvent,
};

local void WaylandSeatCapabilitiesEvent(
    void*               Data,
    struct wl_seat*     Seat,
    u32                 Capabilities
)
{
    wayland_state* Wayland = (wayland_state*)Data;

    if (Wayland->Keyboard)
        wl_keyboard_release(Wayland->Keyboard);

    if (Capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
        Wayland->Keyboard = wl_seat_get_keyboard(Seat);

    if (Wayland->Keyboard)
        wl_keyboard_add_listener(Wayland->Keyboard, &WaylandKeyboardListener, Wayland);
}

local void WaylandSeatNameEvent(
    void*               Data,
    struct wl_seat*     Seat,
    const char*         Name
)
{
}

local struct wl_seat_listener WaylandSeatListener =
{
    .capabilities = &WaylandSeatCapabilitiesEvent,
    .name = &WaylandSeatNameEvent,
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

    if (Wayland->Output)
        wl_output_add_listener(Wayland->Output, &WaylandOutputListener, Wayland);

    if (Wayland->Seat)
        wl_seat_add_listener(Wayland->Seat, &WaylandSeatListener, Wayland);

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

local f32 WaylandGetRefreshRate(wayland_state* Wayland)
{
    return (Wayland->RefreshRate > 0) ? (Wayland->RefreshRate) : (60);
}

local void WaylandPollEvents(wayland_state* Wayland, input_array* Inputs)
{
    Wayland->Inputs = Inputs;
    wl_display_roundtrip(Wayland->Display);
    Wayland->Inputs = 0;
}

local void WaylandPresent(wayland_state* Wayland)
{
    wl_surface_commit(Wayland->Surface);
}

