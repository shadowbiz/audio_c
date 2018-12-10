#pragma once

#define internal static
#define local static
#define global static

#include "editor_platform.h"
#include "editor_sound.h"

#if EDITOR_SLOW
#define Assert(expression) if (!(expression)) { *(int *)0 = 0; }
#else
#define Assert(expression)
#endif

#include "editor_math.h"

#define ArrayCount(array)   (sizeof(array)/sizeof((array)[0]))

#define Kilobytes(bytes)    ((bytes)*1024LL)
#define Megabytes(bytes)    (Kilobytes(bytes)*1024LL)
#define Gigabytes(bytes)    (Megabytes(bytes)*1024LL)
#define Terabytes(bytes)    (Gigabytes(bytes)*1024LL)

#define ToKilobytes(bytes)  ((bytes)/1024LL)
#define ToMegabytes(bytes)  (ToKilobytes(bytes)/1024LL)
#define ToGigabytes(bytes)  (ToMegabytes(bytes)/1024LL)
#define ToTerabytes(bytes)  (ToGigabytes(bytes)/1024LL)

inline ControllerInput *
GetController(Input *input, u32 controllerIndex)
{
    Assert(controllerIndex < ArrayCount(input->Controllers));
    
    auto result = &input->Controllers[controllerIndex];
    return result;
}

struct AppState
{
    i32 GreenOffset;
    i32 BlueOffset;
    
    i32 X;
    i32 Y;
};

