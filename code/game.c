
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

local void GameUpdate(f32 DeltaTime);
local void GameRender(render_array* Renders);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================

local void GameUpdate(f32 DeltaTime)
{
    // NOTE(vak): Nothing here (yet), folks!
}

local void GameRender(render_array* Renders)
{
    ResetRenders(Renders);

    RenderRect(
        Renders,
        100.0f, 100.0f,
        100.0f, 100.0f,
        1.0f, 0.8f, 0.5f, 1.0f
    );
}

