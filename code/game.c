
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

typedef struct
{
    f32 PlayerX;
    f32 PlayerY;
    f32 PlayerDX;
    f32 PlayerDY;
    f32 PlayerDDX;
    f32 PlayerDDY;

    b32 MoveUp;
    b32 MoveDown;
    b32 MoveLeft;
    b32 MoveRight;
} game_state;

local void GameSetup    (game_state* Game);
local void GameUpdate   (game_state* Game, input_array* Inputs, f32 DeltaTime);
local void GameRender   (game_state* Game, render_array* Renders, f32 DeltaTime);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================

local void GameSetup(game_state* Game)
{
    Game->PlayerX = 1.0f;
    Game->PlayerY = 1.0f;
}

local void GameUpdate(game_state* Game, input_array* Inputs, f32 DeltaTime)
{
    for (usize Index = 0; Index < Inputs->EventCount; Index++)
    {
        input_event* Event = Inputs->Events + Index;

        switch (Event->Kind)
        {
            default: break;

            case InputEventKind_Button:
            {
                b32 IsDown = Event->ButtonDown;
                input_button Button = Event->Button;

                switch (Button)
                {
                    default: break;

                    case InputButton_KeyW: Game->MoveUp     = IsDown; break;
                    case InputButton_KeyA: Game->MoveLeft   = IsDown; break;
                    case InputButton_KeyS: Game->MoveDown   = IsDown; break;
                    case InputButton_KeyD: Game->MoveRight  = IsDown; break;
                }
            };
        }
    }

    f32 DirectionX = (f32)((s32)Game->MoveRight - (s32)Game->MoveLeft);
    f32 DirectionY = (f32)((s32)Game->MoveDown  - (s32)Game->MoveUp);

    if (DirectionX && DirectionY)
    {
        DirectionX *= 0.7071067811865475244f;
        DirectionY *= 0.7071067811865475244f;
    }

    f32 Friction = 35.0f;
    f32 Force = Friction * 8.0f;

    Game->PlayerDDX = Force*DirectionX - Friction*Game->PlayerDX;
    Game->PlayerDDY = Force*DirectionY - Friction*Game->PlayerDY;

    f32 ChangeX = Game->PlayerDX * DeltaTime + Game->PlayerDDX * 0.5f*DeltaTime*DeltaTime;
    f32 ChangeY = Game->PlayerDY * DeltaTime + Game->PlayerDDY * 0.5f*DeltaTime*DeltaTime;

    Game->PlayerX += ChangeX;
    Game->PlayerY += ChangeY;

    Game->PlayerDX = ChangeX / DeltaTime;
    Game->PlayerDY = ChangeY / DeltaTime;
}

local void GameRender(game_state* Game, render_array* Renders, f32 DeltaTime)
{
    f32 PredictedPlayerX =
        Game->PlayerX +
        Game->PlayerDX * DeltaTime +
        Game->PlayerDDX * 0.5f*DeltaTime*DeltaTime;

    f32 PredictedPlayerY =
        Game->PlayerY +
        Game->PlayerDY * DeltaTime +
        Game->PlayerDDY * 0.5f*DeltaTime*DeltaTime;

    RenderRect(
        Renders,
        PredictedPlayerX*300.0f, PredictedPlayerY*300.0f,
        100.0f, 100.0f,
        1.0f, 0.8f, 0.5f, 1.0f
    );
}

