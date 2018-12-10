#include "editor.h"
#include "editor_sound.h"

internal void
DrawRectangle(OffscreenBuffer *buffer, vector2 vMin, vector2 vMax, f32 r, f32 g, f32 b) 
{
    i32 minX = RoundF32ToI32(vMin.X);
    i32 minY = RoundF32ToI32(vMin.Y);
    i32 maxX = RoundF32ToI32(vMax.X);
    i32 maxY = RoundF32ToI32(vMax.Y);
    
    if(minX < 0)
    {
        minX = 0;
    }
    
    if(minY < 0)
    {
        minY = 0;
    }
    
    if(maxX > buffer->Width)
    {
        maxX = buffer->Width;
    }
    
    if(maxY > buffer->Height)
    {
        maxY = buffer->Height;
    }
    
    u32 color = ((RoundF32ToU32(r * 255.0f) << 16) |
                 (RoundF32ToU32(g * 255.0f) << 8) |
                 (RoundF32ToU32(b * 255.0f) << 0));
    
    u8 *row = ((u8 *)buffer->Memory +
               minX*buffer->BytesPerPixel +
               minY*buffer->Pitch);
    
    for(int y = minY; y < maxY; ++y)
    {
        u32 *pixel = (u32 *)row;
        for(int x = minX; x < maxX; ++x)
        {
            *pixel++ = color;
        }
        
        row += buffer->Pitch;
    }
}

inline f64
GetNextSineSample(u32 sampleRateInHz, u32 toneHz, f64 *phase)
{
    const f64 cyclesPerSample = (f64)(toneHz) / sampleRateInHz;
    const f64 phaseDelta = cyclesPerSample * TwoPi32;
    
    auto sample = sin(*phase);
    
    *phase += phaseDelta;
    
    if (*phase > TwoPi32)
    {
        *phase -= TwoPi32;
    }
    
    return sample;
}

internal void
FillBufferWithSine(SoundContainer *container, u32 toneHz, f64 *phase)
{
    u8 *pBegin = container->Samples;
    u8 *pEnd   = container->Samples + container->SampleCount;
    
    while (pBegin < pEnd)
    {
        auto sampleF64 = GetNextSineSample(container->SampleRate, toneHz, phase);
        
        for (u32 channel = 0; channel != container->Channels; ++channel)
        {
            switch (container->Encoding)
            {
                case FLOAT_32:
                {
                    F64toF32(sampleF64, (f32 *)pBegin);
                    pBegin += sizeof(f32);
                }
                break;
                case PCM_8:
                {
                    F64toU8(sampleF64, (u8 *)pBegin);
                    pBegin += sizeof(u8);
                    break;
                }
                case PCM_16:
                {
                    F64toI16(sampleF64, (i16 *)pBegin);
                    pBegin += sizeof(i16);
                    break;
                }
                case PCM_24:
                {
                    F64to24Bit(sampleF64, pBegin);
                    pBegin += 3; // 3 bytes
                    break;
                }
            }
        }
    }
}

internal void
MemoryInitialize(AppMemory *memory)
{
    auto appState = (AppState *)memory->PermanentStorage;
    memory->IsInitialized = true;
}

extern "C" GET_SOUND_SAMPLES(GetSoundSamples)
{
    local f64 phase = 0.0;
    FillBufferWithSine(soundContainer, 440, &phase);
}

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    if (!memory->IsInitialized)
    {
        MemoryInitialize(memory);
    }
    auto appState = (AppState *)memory->PermanentStorage;
    
    vector2 zero = Vector2(0, 0);
    vector2 screenSize = Vector2(buffer->Width, buffer->Height);
    DrawRectangle(buffer, zero, screenSize, 1, 1, 0);
    
    vector2 mousePosition = Vector2(input->MouseX, input->MouseY);
    vector2 mouseMax = mousePosition + Vector2(10, 30);
    
    DrawRectangle(buffer, mousePosition, mouseMax, 1, 0, 0);
}

