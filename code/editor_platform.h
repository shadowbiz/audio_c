#pragma once

#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

struct ThreadContext
{
    int Placeholder;
};

#if EDITOR_INTERNAL

struct DebugReadFileResult
{
    u32 ContentsSize;
    void *Contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext *thread, void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DebugPlatformFreeFileMemoryType);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DebugReadFileResult name(ThreadContext *thread, char *filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DebugPlatformReadEntireFileType);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(ThreadContext *thread, char *filename, u32 memorySize, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DebugPlatformWriteEntireFileType);

#endif

struct ButtonState
{
    int HalfTransitionCount;
    b32 EndedDown;
};

struct ControllerInput
{
    b32 IsConnected;
    b32 IsAnalog;
    f32 StickAverageX;
    f32 StickAverageY;

    ButtonState Buttons[12];
};

struct Input
{
    ButtonState MouseButtons[5];
    i32 MouseX;
    i32 MouseY;
    i32 MouseZ;

    ControllerInput Controllers[5];
};

struct OffscreenBuffer
{
    void *Memory;
    i32 Width;
    i32 Height;
    i32 Pitch;
    i32 BytesPerPixel;
};

struct AppMemory
{
    b32 IsInitialized;

    u64 PermanentStorageSize;
    void *PermanentStorage;

    u64 TransientStorageSize;
    void *TransientStorage;

    DebugPlatformWriteEntireFileType *DEBUGPlatformWriteEntireFile;
    DebugPlatformReadEntireFileType *DEBUGPlatformReadEntireFile;
    DebugPlatformFreeFileMemoryType *DEBUGPlatformFreeFileMemory;
};

#include "editor_sound.h"

#define UPDATE_AND_RENDER(name) void name(ThreadContext *thread, AppMemory *memory, Input *input, OffscreenBuffer *buffer)
typedef UPDATE_AND_RENDER(UpdateAndRenderType);
UPDATE_AND_RENDER(UpdateAndRenderStub)
{
}

#define GET_SOUND_SAMPLES(name) void name(ThreadContext *thread, AppMemory *memory, SoundContainer *soundContainer)
typedef GET_SOUND_SAMPLES(GetSoundSamplesType);
GET_SOUND_SAMPLES(GetSoundSamplesStub)
{
}
