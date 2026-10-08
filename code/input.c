
// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

#pragma once

typedef enum
{
    InputEventKind_Nil,
    InputEventKind_Button,
} input_event_kind;

typedef enum
{
    InputButton_Nil = 0,

    InputButton_KeyW,
    InputButton_KeyA,
    InputButton_KeyS,
    InputButton_KeyD,

    InputButton_KeyUp,
    InputButton_KeyDown,
    InputButton_KeyLeft,
    InputButton_KeyRight,
} input_button;

typedef struct
{
    input_event_kind    Kind;

    // NOTE(vak): InputEventKind_Button
    b32                 ButtonDown;
    input_button        Button;
} input_event;

typedef struct
{
    u32             MaxEventCount;
    u32             EventCount;
    input_event*    Events;
} input_array;

local void ResetInputs(input_array* Input);

local void AddInputEventButton(
    input_array*    Input,
    input_button    Button,
    b32             IsDown
);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================

local void ResetInputs(input_array* Inputs)
{
    Inputs->EventCount = 0;
}

local void AddInputEventButton(
    input_array*    Inputs,
    input_button    Button,
    b32             IsDown
)
{
    if (!Inputs)
        return;

    if (Button == InputButton_Nil)
        return;

    if (Inputs->EventCount < Inputs->MaxEventCount)
    {
        input_event* Event = Inputs->Events + Inputs->EventCount++;

        Event->Kind         = InputEventKind_Button;
        Event->Button       = Button;
        Event->ButtonDown   = IsDown;
    }
}

