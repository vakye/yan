
#pragma once

// ============================================================================
// NOTE(vak): Cheatsheet
// ============================================================================

local usize GetWallClock        (void);
local f64   GetSecondsElapsed   (usize From, usize To);
local void  Wait                (f64 Seconds);
local usize WriteStdOut         (void* Data, usize Size);
local usize WriteStdErr         (void* Data, usize Size);
local void  Exit                (u8 Code);

#define ExitErrorIfNot(Expression) if (!(Expression)) Exit(1)

// ============================================================================
// NOTE(vak): Implementation is selected by including one of the
//            following files depending on the underlying platform:
//
//      + Linux: linux_platform.c
//
// ============================================================================

