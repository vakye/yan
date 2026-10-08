
#pragma once

// ============================================================================
// NOTE(vak): Syscall cheatsheet
// ============================================================================

#define STDOUT_FILENO (1)
#define STDERR_FILENO (2)

enum
{
#if ARCHITECTURE_X64
    SyscallNR_Write     = 1,
    SyscallNR_ExitGroup = 231,
#else
    #error Linux syscall numbers are not defined for this architecture
#endif
};

local usize LinuxSyscallFull(usize NR, usize Args[6]);

#define LinuxSyscall0(NR, ...) LinuxSyscallFull(NR, (usize[6]){0})
#define LinuxSyscallX(NR, ...) LinuxSyscallFull(NR, (usize[6]){__VA_ARGS__})

// ============================================================================
// NOTE(vak): Implementation of platform.c
// ============================================================================

#include <dlfcn.h>

local usize WriteStdOut(void* Data, usize Size)
{
    ssize Result = LinuxSyscallX(SyscallNR_Write, STDOUT_FILENO, (usize)Data, Size);
    return Maximum(0, Result);
}

local usize WriteStdErr(void* Data, usize Size)
{
    ssize Result = LinuxSyscallX(SyscallNR_Write, STDERR_FILENO, (usize)Data, Size);
    return Maximum(0, Result);
}

local void Exit(u8 Code)
{
    LinuxSyscallX(SyscallNR_ExitGroup, Code);
}

// ============================================================================
// NOTE(vak): Syscall implementation
// ============================================================================

local usize LinuxSyscallFull(usize NR, usize Args[6])
{
    usize Result = 0;

#if ARCHITECTURE_X64
    register usize R10 __asm__("r10") = Args[3];
    register usize R8  __asm__("r8")  = Args[4];
    register usize R9  __asm__("r9")  = Args[5];

    __asm__ volatile
    (
        "syscall" :
        "=a"(Result) :
        "a"(NR),
        "D"(Args[0]),
        "S"(Args[1]),
        "d"(Args[2]),
        "r"(R10),
        "r"(R8),
        "r"(R9) :
        "memory", "rcx", "r11"
    );
#else
    #error LinuxSyscallFull unimplemented for this architecture
#endif

    return (Result);
}

// ============================================================================
// NOTE(vak): Entry point
// ============================================================================

__attribute__((naked))
void EntryPoint(void)
{
#if ARCHITECTURE_X64
    __asm__ volatile
    (
        "mov 0(%rsp),           %edi\n"
        "lea 8(%rsp),           %rsi\n"
        "lea 16(%rsp, %rdi, 8), %rdx\n"
        "call LinuxEntry\n"
    );
#else
    #error Entry point for Linux unimplemented for this architecture
#endif
}

