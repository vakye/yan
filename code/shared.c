
#pragma once

// ============================================================================
// NOTE(vak): Architecture
// ============================================================================

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
    #define ARCHITECTURE_X64 (1)
#else
    #error Unknown architecture
#endif

#if !defined(ARCHITECTURE_X64)
    #define ARCHITECTURE_X64 (0)
#endif

// ============================================================================
// NOTE(vak): Keywords
// ============================================================================

#define local static
#define persist static

// ============================================================================
// NOTE(vak): Macros
// ============================================================================

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Minimum(A, B) ((A) < (B) ? (A) : (B))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))

#define Clamp(Min, Value, Max) Maximum(Min, Minimum(Max, Value))

#define KB(Amount) ((ssize)(Amount) << 10)
#define MB(Amount) ((ssize)(Amount) << 20)
#define GB(Amount) ((ssize)(Amount) << 30)
#define TB(Amount) ((ssize)(Amount) << 40)

// ============================================================================
// NOTE(vak): Types
// ============================================================================

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef s64 ssize;
typedef u64 usize;

typedef float f32;
typedef double f64;

typedef u8 b8;
typedef u32 b32;

// ============================================================================
// NOTE(vak): Constants
// ============================================================================

#define true (1)
#define false (0)

#define S8_MIN ((s8)(0x80))
#define S16_MIN ((s16)(0x8000))
#define S32_MIN ((s32)(0x80000000))
#define S64_MIN ((s64)(0x8000000000000000))

#define S8_MAX ((s8)(0x7f))
#define S16_MAX ((s16)(0x7fff))
#define S32_MAX ((s32)(0x7fffffff))
#define S64_MAX ((s64)(0x7fffffffffffffff))

#define U8_MAX ((u8)(0xff))
#define U16_MAX ((u16)(0xffff))
#define U32_MAX ((u32)(0xffffffff))
#define U64_MAX ((u64)(0xffffffffffffffff))

#define SSIZE_MIN S64_MIN
#define SSIZE_MAX S64_MAX
#define USIZE_MAX U64_MAX

// ============================================================================
// NOTE(vak): Strings
// ============================================================================

typedef struct
{
    char* Data;
    usize Size;
} string;

#define Str(Literal) (string){Literal, sizeof(Literal) - 1}
#define StrData(Data, Size) (string){Data, Size}

local string CString(const char* Data)
{
    string Result = {0};

    if (Data)
    {
        Result.Data = (char*)Data;

        while (Data[Result.Size] != '\0')
            Result.Size++;
    }

    return (Result);
}

local b32 StringEquals(string A, string B)
{
    b32 Result = (A.Size == B.Size);

    if (Result)
    {
        for (usize Index = 0; Index < B.Size; Index++)
        {
            if (A.Data[Index] != B.Data[Index])
            {
                Result = false;
                break;
            }
        }
    }

    return (Result);
}

