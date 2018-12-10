#pragma once

#include "editor.h"

#include <windows.h>
#include <initguid.h>
#include <avrt.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

struct Win32OffscreenBuffer
{
    BITMAPINFO Info;
    void      *Memory;
    i32        Width;
    i32        Height;
    i32        Pitch;
    i32        BytesPerPixel;
};

struct Win32WindowDimension
{
    u32        Width;
    u32        Height;
};

struct Win32AppCode
{
    HMODULE              AppCodeDLL;
    FILETIME             DLLLastWriteTime;
    
    UpdateAndRenderType *UpdateAndRender;
    GetSoundSamplesType *GetSoundSamples;
    
    b32                  IsValid;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct Win32State
{
    u64    TotalSize;
    void  *AppMemoryBlock;
    
    HANDLE RecordingHandle;
    i32    InputRecordingIndex;
    
    HANDLE PlaybackHandle;
    i32    InputPlayingIndex;
    
    char   EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char  *OnePastLastEXEFileNameSlash;
};

#include <atomic>

struct AtomicI16
{
    std::atomic<i16> x;
};

struct AtomicB32
{
    std::atomic<b32> x;
};

struct AtomicF32
{
    std::atomic<f32> x;
};

struct AtomicFlag
{
    std::atomic_flag x;
};

#define SOUNDIO_ATOMIC_LOAD(a) (a.x.load())
#define SOUNDIO_ATOMIC_FETCH_ADD(a, delta) (a.x.fetch_add(delta))
#define SOUNDIO_ATOMIC_STORE(a, value) (a.x.store(value))
#define SOUNDIO_ATOMIC_EXCHANGE(a, value) (a.x.exchange(value))
#define SOUNDIO_ATOMIC_FLAG_TEST_AND_SET(a) (a.x.test_and_set())
#define SOUNDIO_ATOMIC_FLAG_CLEAR(a) (a.x.clear())
#define SOUNDIO_ATOMIC_FLAG_INIT ATOMIC_FLAG_INIT
