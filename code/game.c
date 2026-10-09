
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

typedef struct
{
    f32 X;
    f32 Y;
    f32 DX;
    f32 DY;
    f32 DDX;
    f32 DDY;

    b32 MoveUp;
    b32 MoveDown;
    b32 MoveLeft;
    b32 MoveRight;
} game_state;

local void GameSetup    (game_state* Game);
local void GameUpdate   (game_state* Game, input_array* Inputs, f32 DeltaTime);
local void GameRender   (game_state* Game, render_array* Renders, f32 PredictDeltaTime);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================

local void GameSetup(game_state* Game)
{
    Game->X = 1.0f;
    Game->Y = 1.0f;
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

    f32 Friction = 50.0f;
    f32 Force = Friction * 2.0f;

    Game->DDX = Force * DirectionX - Friction * Game->DX;
    Game->DDY = Force * DirectionY - Friction * Game->DY;

    f32 ChangeX = DeltaTime*(Game->DX + Game->DDX*0.5f*DeltaTime);
    f32 ChangeY = DeltaTime*(Game->DY + Game->DDY*0.5f*DeltaTime);

    Game->X += ChangeX;
    Game->Y += ChangeY;

    Game->DX = ChangeX / DeltaTime;
    Game->DY = ChangeY / DeltaTime;
}

local void GameRender(game_state* Game, render_array* Renders, f32 PredictDeltaTime)
{
    f32 DeltaTime = PredictDeltaTime;

    f32 PredictedX = Game->X + DeltaTime*(Game->DX + Game->DDX*0.5f*DeltaTime);
    f32 PredictedY = Game->Y + DeltaTime*(Game->DY + Game->DDY*0.5f*DeltaTime);

    RenderRect(
        Renders,
        PredictedX*300.0f, PredictedY*300.0f,
        100.0f, 100.0f,
        1.0f, 0.8f, 0.5f, 1.0f
    );
}

