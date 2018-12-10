#include "win32_editor.h"

#include "tools.h"
#include "editor_wasapi.cpp"
#include "editor_memory.h"

global b32 SoundIsPlaying;
global b32 AppRunning;
global Win32OffscreenBuffer GlobalBackbuffer;
global i64 GlobalPerfCountFrequency;

internal void
Win32GetEXEFileName(Win32State *state)
{
    DWORD sizeOfFilename = GetModuleFileNameA(0, state->EXEFileName, sizeof(state->EXEFileName));
    state->OnePastLastEXEFileNameSlash = state->EXEFileName;
    for (char *scan = state->EXEFileName; *scan; ++scan)
    {
        if (*scan == '\\')
        {
            state->OnePastLastEXEFileNameSlash = scan + 1;
        }
    }
}

internal void
Win32BuildEXEPathFileName(Win32State *state, char *fileName, int destCount, char *dest)
{
    CatStrings(state->OnePastLastEXEFileNameSlash - state->EXEFileName, state->EXEFileName,
               StringLength(fileName), fileName, destCount, dest);
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if (memory)
    {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    DebugReadFileResult result = {};

    auto fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            u32 fileSize32 = TruncateU64(fileSize.QuadPart);
            result.Contents = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (result.Contents)
            {
                DWORD bytesRead;
                if (ReadFile(fileHandle, result.Contents, fileSize32, &bytesRead, 0) &&
                    (fileSize32 == bytesRead))
                {
                    result.ContentsSize = fileSize32;
                }
                else
                {
                    DEBUGPlatformFreeFileMemory(thread, result.Contents);
                    result.Contents = 0;
                }
            }
            else
            {
                //TODO: Log
            }
        }
        else
        {
            //TODO: Log
        }

        CloseHandle(fileHandle);
    }
    else
    {
        //TODO: Log
    }

    return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    b32 result = false;

    auto fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
        {
            result = (bytesWritten == memorySize);
        }
        else
        {
            //TODO: Log
        }

        CloseHandle(fileHandle);
    }
    else
    {
        //TODO: Log
    }

    return result;
}

inline FILETIME
Win32GetLastWriteTime(char *filename)
{
    FILETIME lastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        lastWriteTime = data.ftLastWriteTime;
    }

    return lastWriteTime;
}

internal Win32AppCode
Win32LoadAppCode(char *sourceDLLName, char *tempDLLName)
{
    Win32AppCode result = {};

    result.DLLLastWriteTime = Win32GetLastWriteTime(sourceDLLName);
    CopyFile(sourceDLLName, tempDLLName, FALSE);

    result.AppCodeDLL = LoadLibraryA(tempDLLName);
    if (result.AppCodeDLL)
    {
        result.UpdateAndRender = (UpdateAndRenderType *)
            GetProcAddress(result.AppCodeDLL, "UpdateAndRender");

        result.GetSoundSamples = (GetSoundSamplesType *)
            GetProcAddress(result.AppCodeDLL, "GetSoundSamples");

        result.IsValid = (result.UpdateAndRender &&
                          result.GetSoundSamples);
    }

    if (!result.IsValid)
    {
        result.UpdateAndRender = UpdateAndRenderStub;
        result.GetSoundSamples = GetSoundSamplesStub;
    }

    return result;
}

internal void
Win32UnloadAppCode(Win32AppCode *appCode)
{
    if (appCode->AppCodeDLL)
    {
        FreeLibrary(appCode->AppCodeDLL);
        appCode->AppCodeDLL = 0;
    }

    appCode->IsValid = false;
    appCode->UpdateAndRender = UpdateAndRenderStub;
    appCode->GetSoundSamples = GetSoundSamplesStub;
}

Win32WindowDimension
Win32GetWindowDimension(HWND window)
{
    Win32WindowDimension result;

    RECT clientRect;
    GetClientRect(window, &clientRect);
    result.Width = clientRect.right - clientRect.left;
    result.Height = clientRect.bottom - clientRect.top;

    return result;
}

internal void
Win32ResizeDIBSection(Win32OffscreenBuffer *buffer, int width, int height)
{
    if (buffer->Memory)
    {
        VirtualFree(buffer->Memory, 0, MEM_RELEASE);
    }

    buffer->Width = width;
    buffer->Height = height;

    i32 bytesPerPixel = 4;
    buffer->BytesPerPixel = bytesPerPixel;

    buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
    buffer->Info.bmiHeader.biWidth = buffer->Width;
    buffer->Info.bmiHeader.biHeight = -buffer->Height;
    buffer->Info.bmiHeader.biPlanes = 1;
    buffer->Info.bmiHeader.biBitCount = 32;
    buffer->Info.bmiHeader.biCompression = BI_RGB;

    int bitmapMemSize = (buffer->Width * buffer->Height) * bytesPerPixel;
    buffer->Memory = VirtualAlloc(0, bitmapMemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    buffer->Pitch = width * bytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(Win32OffscreenBuffer *buffer,
                           HDC deviceContext,
                           int windowWidth, int windowHeight)
{
    StretchDIBits(deviceContext,
                  0, 0, buffer->Width, buffer->Height,
                  0, 0, buffer->Width, buffer->Height,
                  buffer->Memory, &buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND window, UINT message,
                        WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message)
    {
    case WM_CLOSE:
    {
        AppRunning = false;
    }
    break;

    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
    }
    break;

    case WM_DESTROY:
    {
        AppRunning = false;
    }
    break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        Assert(!"Keyboard input came in through a non-dispatch message!");
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        auto deviceContext = BeginPaint(window, &paint);
        Win32WindowDimension dimension = Win32GetWindowDimension(window);
        Win32DisplayBufferInWindow(&GlobalBackbuffer, deviceContext,
                                   dimension.Width, dimension.Height);
        EndPaint(window, &paint);
    }
    break;

    default:
    {
        // OutputDebugStringA("default\n");
        result = DefWindowProcA(window, message, wParam, lParam);
    }
    break;
    }

    return result;
}

internal void
Win32ProcessKeyboardMessage(ButtonState *newState, b32 isDown)
{
    if (newState->EndedDown != isDown)
    {
        newState->EndedDown = isDown;
        ++newState->HalfTransitionCount;
    }
}

internal void
Win32StopSound()
{
    SoundIsPlaying = false;
}

internal void
Win32StartSound()
{
    //SoundIsPlaying = true;
}

internal void
Win32ProcessPendingMessages(ControllerInput *keyboardController)
{
    MSG message;

    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch (message.message)
        {
        case WM_QUIT:
        {
            AppRunning = false;
        }
        break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = (u32)message.wParam;
            b32 wasDown = ((message.lParam & (1 << 30)) != 0);
            b32 isDown = ((message.lParam & (1 << 31)) == 0);
            if (wasDown != isDown)
            {
                switch (VKCode)
                {
                case VK_SPACE:
                    if (SoundIsPlaying)
                    {
                        Win32StopSound();
                    }
                    else
                    {
                        Win32StartSound();
                    }
                    break;
                }
            }

            b32 altKeyWasDown = (message.lParam & (1 << 29));
            if ((VKCode == VK_F4) && altKeyWasDown)
            {
                AppRunning = false;
            }
        }
        break;

        default:
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        break;
        }
    }
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline f32
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = ((f32)(end.QuadPart - start.QuadPart) /
                  (f32)GlobalPerfCountFrequency);
    return result;
}

DWORD WINAPI
AudioThread(LPVOID lpParameter)
{
    auto wasapiRenderer = (WasapiRenderer *)lpParameter;
    OutputDebugStringA("Thread Started!\n");

    SoundIsPlaying = true;

    SoundOutputBuffer soundOutputBuffer = {};

    SoundContainer playbackSoundContainer = {};

    playbackSoundContainer.SampleRate = wasapiRenderer->SampleRate;
    playbackSoundContainer.Channels = wasapiRenderer->Channels;

    auto appMemory = wasapiRenderer->Memory;

    //auto tempMemoryPart = PartitionMake(appMemory->TransientStorage, Megabytes(100));

    IAudioClient_Start(wasapiRenderer->AudioClient);

    while (true)
    {
        if (!SoundIsPlaying)
        {
            break;
        }

        u32 numFramesPadding = 0;

        HRESULT hresult;

        /*
        hresult = wasapiRenderer->AudioClient->GetCurrentPadding(&numFramesPadding);
        if (FAILED(hresult))
        {
            OutputDebugStringA("Couldn't obtain current padding.");
            break;
        }
        */

        //u32 numFramesAvailable = wasapiRenderer->BufferSizeInFrames - numFramesPadding;

        BYTE *sampleBuffer;

        hresult = IAudioRenderClient_GetBuffer(wasapiRenderer->RenderClient,
                                               wasapiRenderer->BufferSizeInFrames, &sampleBuffer);
        if (FAILED(hresult))
        {
            OutputDebugStringA("Couldn't obtain buffer.");
            break;
        }

        playbackSoundContainer.SampleCount = wasapiRenderer->BufferSizeInFrames * wasapiRenderer->FrameSizeInBytes;
        playbackSoundContainer.Samples = sampleBuffer;
        playbackSoundContainer.Encoding = wasapiRenderer->Encoding;

        ThreadContext thread = {};

        wasapiRenderer->GetSoundSamples(&thread, wasapiRenderer->Memory, &playbackSoundContainer);

        IAudioRenderClient_ReleaseBuffer(wasapiRenderer->RenderClient,
                                         wasapiRenderer->BufferSizeInFrames, 0 /*flags*/);

        DWORD waitEvent = WaitForSingleObject(wasapiRenderer->CallbackEvent, 500);

        if (waitEvent != WAIT_OBJECT_0)
        {
            break;
        }
    }

    IAudioClient_Stop(wasapiRenderer->AudioClient);

    OutputDebugStringA("Releasing WASAPI\n");
    Win32ReleaseWASAPI(wasapiRenderer);

    return 0;
}

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prevInstance,
        LPSTR lcmdLine, int cmdShow)
{

    Win32State win32State = {};

    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    GlobalPerfCountFrequency = perfCountFrequencyResult.QuadPart;

    Win32GetEXEFileName(&win32State);

    char sourceAppCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&win32State, "editor.dll",
                              sizeof(sourceAppCodeDLLFullPath), sourceAppCodeDLLFullPath);

    char tempAppCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&win32State, "editor_temp.dll",
                              sizeof(tempAppCodeDLLFullPath), tempAppCodeDLLFullPath);

    UINT desiredSchedulerMS = 1;
    b32 sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);

    WNDCLASS windowClass = {};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Win32MainWindowCallback;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "AudioEditorClass";

    if (RegisterClassA(&windowClass))
    {
        auto window = CreateWindowEx(0, windowClass.lpszClassName, "Audio Editor",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                     0, 0, instance, 0);
        if (window)
        {
            int monitorRefreshHz = 60;
            HDC refreshDC = GetDC(window);
            int win32RefreshRate = GetDeviceCaps(refreshDC, VREFRESH);
            ReleaseDC(window, refreshDC);

            if (win32RefreshRate > 1)
            {
                monitorRefreshHz = win32RefreshRate;
            }

            f32 gameUpdateHz = (monitorRefreshHz / 2.0f);
            f32 targetSecondsPerFrame = 1.0f / (f32)gameUpdateHz;

            AppRunning = true;

#if EDITOR_INTERNAL
            auto baseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID baseAddress = 0;
#endif

            AppMemory appMemory = {};
            appMemory.PermanentStorageSize = Megabytes(2);
            appMemory.TransientStorageSize = Megabytes(512);
            appMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
            appMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
            appMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;

            u64 totalSize = appMemory.PermanentStorageSize + appMemory.TransientStorageSize;
            appMemory.PermanentStorage = VirtualAlloc(baseAddress, totalSize,
                                                      MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            appMemory.TransientStorage = ((u8 *)appMemory.PermanentStorage +
                                          appMemory.PermanentStorageSize);

            auto app = Win32LoadAppCode(sourceAppCodeDLLFullPath, tempAppCodeDLLFullPath);

            CoInitializeEx(0, COINIT_APARTMENTTHREADED);

            WasapiRenderer wasapiRenderer = {};
            b32 soundInitialized = Win32InitWASAPI(&wasapiRenderer);

            wasapiRenderer.Memory = &appMemory;
            wasapiRenderer.GetSoundSamples = app.GetSoundSamples;

            if (appMemory.PermanentStorage && appMemory.TransientStorage &&
                soundInitialized)
            {
                Input input[2] = {};
                Input *newInput = &input[0];
                Input *oldInput = &input[1];

                auto lastCounter = Win32GetWallClock();
                auto flipWallClock = Win32GetWallClock();

                DWORD threadID;
                HANDLE audioThreadHandle = CreateThread(0, 0, AudioThread, &wasapiRenderer, 0, &threadID);

                u64 lastCycleCount = __rdtsc();

                while (AppRunning)
                {
                    auto oldKeyboardController = GetController(oldInput, 0);
                    auto newKeyboardController = GetController(newInput, 0);
                    *newKeyboardController = {};
                    newKeyboardController->IsConnected = true;

                    auto arrayCount = ArrayCount(newKeyboardController->Buttons);
                    for (int buttonIndex = 0; buttonIndex != arrayCount; ++buttonIndex)
                    {
                        newKeyboardController->Buttons[buttonIndex].EndedDown =
                            oldKeyboardController->Buttons[buttonIndex].EndedDown;
                    }

                    Win32ProcessPendingMessages(newKeyboardController);

                    POINT mouseP;
                    GetCursorPos(&mouseP);
                    ScreenToClient(window, &mouseP);

                    newInput->MouseX = mouseP.x;
                    newInput->MouseY = mouseP.y;
                    newInput->MouseZ = 0; // TODO: Support mousewheel?

                    Win32ProcessKeyboardMessage(&newInput->MouseButtons[0],
                                                GetKeyState(VK_LBUTTON) & (1 << 15));
                    Win32ProcessKeyboardMessage(&newInput->MouseButtons[1],
                                                GetKeyState(VK_MBUTTON) & (1 << 15));
                    Win32ProcessKeyboardMessage(&newInput->MouseButtons[2],
                                                GetKeyState(VK_RBUTTON) & (1 << 15));
                    Win32ProcessKeyboardMessage(&newInput->MouseButtons[3],
                                                GetKeyState(VK_XBUTTON1) & (1 << 15));
                    Win32ProcessKeyboardMessage(&newInput->MouseButtons[4],
                                                GetKeyState(VK_XBUTTON2) & (1 << 15));

                    ThreadContext thread = {};

                    OffscreenBuffer buffer = {};
                    buffer.Memory = GlobalBackbuffer.Memory;
                    buffer.Width = GlobalBackbuffer.Width;
                    buffer.Height = GlobalBackbuffer.Height;
                    buffer.Pitch = GlobalBackbuffer.Pitch;
                    buffer.BytesPerPixel = GlobalBackbuffer.BytesPerPixel;

                    /*
                    char mouseBuffer[256];
                    _snprintf_s(mouseBuffer, sizeof(mouseBuffer),
                                "X= %d, Y=  %d W=%d, H=%d \n", mouseP.x, mouseP.y,
                               buffer.Width, buffer.Height);
                    OutputDebugStringA(mouseBuffer);
                    */

                    if (app.UpdateAndRender)
                    {
                        app.UpdateAndRender(&thread, &appMemory, newInput, &buffer);
                    }

                    auto workCounter = Win32GetWallClock();
                    auto workSecondsElapsed = Win32GetSecondsElapsed(lastCounter, workCounter);

                    f32 secondsElapsedForFrame = workSecondsElapsed;
                    if (secondsElapsedForFrame < targetSecondsPerFrame)
                    {
                        if (sleepIsGranular)
                        {
                            auto sleepMS = (DWORD)(1000.0f * (targetSecondsPerFrame -
                                                              secondsElapsedForFrame));
                            if (sleepMS > 0)
                            {
                                Sleep(sleepMS);
                            }
                        }

                        auto testSecondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter,
                                                                                 Win32GetWallClock());
                        if (testSecondsElapsedForFrame < targetSecondsPerFrame)
                        {
                            // TODO: Log
                        }

                        while (secondsElapsedForFrame < targetSecondsPerFrame)
                        {
                            secondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter,
                                                                            Win32GetWallClock());
                        }
                    }
                    else
                    {
                        // TODO: Log
                    }

                    auto endCounter = Win32GetWallClock();
                    auto msPerFrame = 1000.0f * Win32GetSecondsElapsed(lastCounter, endCounter);
                    lastCounter = endCounter;

                    auto dimension = Win32GetWindowDimension(window);
                    auto deviceContext = GetDC(window);

                    Win32DisplayBufferInWindow(&GlobalBackbuffer, deviceContext,
                                               dimension.Width, dimension.Height);
                    ReleaseDC(window, deviceContext);

                    flipWallClock = Win32GetWallClock();

                    Input *temp = newInput;
                    newInput = oldInput;
                    oldInput = temp;

                    u64 endCycleCount = __rdtsc();
                    u64 cyclesElapsed = endCycleCount - lastCycleCount;
                    lastCycleCount = endCycleCount;

                    f64 fps = 0.0f;
                    f64 mcpf = ((f64)cyclesElapsed / (1000.0f * 1000.0f));

                    //char fpsBuffer[256];
                    //_snprintf_s(fpsBuffer, sizeof(fpsBuffer),
                    //            "%.02fms/f,  %.02ff/s,  %.02fmc/f\n", msPerFrame, fps, mcpf);
                    //OutputDebugStringA(fpsBuffer);
                }

                CloseHandle(audioThreadHandle);
            }
        }
        else
        {
            //TODO: Log
        }
    }
    else
    {
        //TODO: Log
    }

    return 0;
}