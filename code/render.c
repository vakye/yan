
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

typedef struct
{
    f32 X, Y; // NOTE(vak): Top-left corner
    f32 W, H;
    f32 R, G, B, A;
} render_rect;

typedef struct
{
    usize           MaxRectCount;
    usize           RectCount;
    render_rect*    Rects;
} render_array;

local void ResetRenders(render_array* Renders);

local void RenderRect(
    render_array* Renders,
    f32 X, f32 Y,
    f32 W, f32 H,
    f32 R, f32 G, f32 B, f32 A
);

// ============================================================================
// NOTE(vak): Implementation
// ============================================================================

local void ResetRenders(render_array* Renders)
{
    Renders->RectCount = 0;
}

local void RenderRect(
    render_array* Renders,
    f32 X, f32 Y,
    f32 W, f32 H,
    f32 R, f32 G, f32 B, f32 A
)
{
    if (Renders->RectCount < Renders->MaxRectCount)
    {
        render_rect* Rect = Renders->Rects + Renders->RectCount++;

        Rect->X = X;
        Rect->Y = Y;
        Rect->W = W;
        Rect->H = H;

        Rect->R = R;
        Rect->G = G;
        Rect->B = B;
        Rect->A = A;
    }
}

