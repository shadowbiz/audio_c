#include "editor_wasapi.h"

internal void
Win32ReleaseWASAPI(WasapiRenderer *sRenderer)
{
    if (sRenderer->Task != 0)
    {
        AvRevertMmThreadCharacteristics(sRenderer->Task);
    }

    if (sRenderer->Enumerator != 0)
    {
        IMMDeviceEnumerator_Release(sRenderer->Enumerator);
        sRenderer->Enumerator = 0;
    }

    if (sRenderer->Device != 0)
    {
        IUnknown_Release(sRenderer->Device);
        sRenderer->Device = 0;
    }

    if (sRenderer->AudioClient != 0)
    {
        IUnknown_Release(sRenderer->AudioClient);
        sRenderer->AudioClient = 0;
    }

    if (sRenderer->RenderClient != 0)
    {
        IUnknown_Release(sRenderer->RenderClient);
        sRenderer->RenderClient = 0;
    }

    if (sRenderer->CallbackEvent != 0)
    {
        CloseHandle(sRenderer->CallbackEvent);
        sRenderer->CallbackEvent = 0;
    }

    CoUninitialize();
}

internal void
DetermineAudioEncoding(WasapiRenderer *sRenderer, WAVEFORMATEXTENSIBLE *waveFormat)
{
    Assert(waveFormat->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE);
    b32 isPCM = IS_EQUAL_GUID(&waveFormat->SubFormat, &SOUNDIO_KSDATAFORMAT_SUBTYPE_PCM);
    b32 isFloat = IS_EQUAL_GUID(&waveFormat->SubFormat, &SOUNDIO_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);

    sRenderer->Channels = waveFormat->Format.nChannels;
    sRenderer->FrameSizeInBytes = waveFormat->Format.nBlockAlign;
    sRenderer->SampleRate = waveFormat->Format.nSamplesPerSec;

    if (waveFormat->Samples.wValidBitsPerSample == waveFormat->Format.wBitsPerSample)
    {
        if (waveFormat->Format.wBitsPerSample == 8)
        {
            if (isPCM)
            {
                sRenderer->Encoding = PCM_8;
                return;
            }
        }
        else if (waveFormat->Format.wBitsPerSample == 16)
        {
            if (isPCM)
            {
                sRenderer->Encoding = PCM_16;
                return;
            }
        }
        else if (waveFormat->Format.wBitsPerSample == 32)
        {
            if (isPCM)
            {
                sRenderer->Encoding = PCM_32;
                return;
            }
            else if (isFloat)
            {
                sRenderer->Encoding = FLOAT_32;
                return;
            }
        }
        else if (waveFormat->Format.wBitsPerSample == 64)
        {
            sRenderer->Encoding = FLOAT_24;
            return;
            //if (isFloat) return SoundIoFormatFloat64LE;
        }
    }
    else if (waveFormat->Format.wBitsPerSample == 32 &&
             waveFormat->Samples.wValidBitsPerSample == 24)
    {
        sRenderer->Encoding = PCM_24_IN_32;
        return;
    }

    sRenderer->Encoding = UNSUPPORTED;
}

internal b32
Win32InitWASAPI(WasapiRenderer *wasapiRenderer)
{

    HRESULT hresult = CoCreateInstance(CLSID_MMDEVICEENUMERATOR,
                                       0,
                                       CLSCTX_ALL,
                                       IID_IMMDEVICEENUMERATOR,
                                       (void **)(&wasapiRenderer->Enumerator));

    if (FAILED(hresult))
    {
        OutputDebugStringA("Failed to acquire device enumerator.\n");
        goto Exit;
    }

    hresult = IMMDeviceEnumerator_GetDefaultAudioEndpoint(wasapiRenderer->Enumerator,
                                                          eRender,
                                                          eMultimedia,
                                                          &wasapiRenderer->Device);

    if (FAILED(hresult))
    {
        OutputDebugStringA("Failed to acquire default audio render device!\n");
        goto Exit;
    }

    IPropertyStore *propertyStore;
    hresult = IMMDevice_OpenPropertyStore(wasapiRenderer->Device,
                                          STGM_READ,
                                          &propertyStore);
    if (FAILED(hresult))
    {
        OutputDebugStringA("Couldn't open property store for device!\n");
        goto Exit;
    }

    PROPVARIANT varAudioFormat;
    PropVariantInit(&varAudioFormat);
    hresult = IPropertyStore_GetValue(propertyStore,
                                      PKEY_AudioEngine_DeviceFormat,
                                      &varAudioFormat);
    if (FAILED(hresult))
    {
        OutputDebugStringA("Couldn't determine current format of the device.");
        goto Exit;
    }

    hresult = IMMDevice_Activate(wasapiRenderer->Device,
                                 IID_IAUDIOCLIENT,
                                 CLSCTX_ALL,
                                 0,
                                 (void **)&wasapiRenderer->AudioClient);
    if (FAILED(hresult))
    {
        OutputDebugStringA("Failed to acquire default audio client.\n");
        goto Exit;
    }

    WAVEFORMATEXTENSIBLE *waveFormat = (WAVEFORMATEXTENSIBLE *)varAudioFormat.blob.pBlobData;

    DetermineAudioEncoding(wasapiRenderer, waveFormat);

    switch (wasapiRenderer->Encoding)
    {
    case FLOAT_32:
        OutputDebugStringA("FLOATING_POINT\n");
        break;
    case PCM_8:
        OutputDebugStringA("PCM_8\n");
        break;
    case PCM_16:
        OutputDebugStringA("PCM_16\n");
        break;
    case PCM_24:
        OutputDebugStringA("PCM_24\n");
        break;
    case PCM_24_IN_32:
        OutputDebugStringA("PCM_24_IN_32\n");
        break;
    case PCM_32:
        OutputDebugStringA("PCM_32\n");
        break;
    case UNSUPPORTED:
        OutputDebugStringA("UNSUPPORTED\n");
        break;
    }

    i64 devicePeriod;
    hresult = IAudioClient_GetDevicePeriod(wasapiRenderer->AudioClient,
                                           0,
                                           &devicePeriod);

    if (FAILED(hresult))
    {
        OutputDebugStringA("Couldn't determine device period.\n");
        goto Exit;
    }

    i64 bufferLengthInPeriod = devicePeriod * 2;

    hresult = IAudioClient_Initialize(wasapiRenderer->AudioClient,
                                      AUDCLNT_SHAREMODE_EXCLUSIVE,
                                      //AUDCLNT_SHAREMODE_SHARED,
                                      AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                      bufferLengthInPeriod,
                                      bufferLengthInPeriod,
                                      (WAVEFORMATEX *)waveFormat,
                                      0);
    if (FAILED(hresult))
    {
        OutputDebugStringA("Could not initialize stream.\n");
        goto Exit;
    }

    REFERENCE_TIME latency;
    hresult = IAudioClient_GetStreamLatency(wasapiRenderer->AudioClient, &latency);
    if (FAILED(hresult))
    {
        OutputDebugStringA("Couldn't get latency\n");
        goto Exit;
    }

    char formatBuffer[256];
    _snprintf_s(formatBuffer, sizeof(formatBuffer),
                "Ch= %d, Samples= %d BitsPerSample=%d Latency = %.02fms\n",
                wasapiRenderer->Channels,
                wasapiRenderer->SampleRate,
                waveFormat->Format.wBitsPerSample,
                MilliSecondsFromReferenceTime(latency));

    //CoTaskMemFree(waveFormat);

    OutputDebugStringA(formatBuffer);

    wasapiRenderer->CallbackEvent = CreateEvent(0, 0, 0, 0);
    if (wasapiRenderer->CallbackEvent == 0)
    {
        OutputDebugStringA("Could not create event.\n");
        goto Exit;
    }

    hresult = IAudioClient_SetEventHandle(wasapiRenderer->AudioClient, wasapiRenderer->CallbackEvent);

    if (FAILED(hresult))
    {
        OutputDebugStringA("Could not set event on audio client.\n");
        goto Exit;
    }

    hresult = IAudioClient_GetService(wasapiRenderer->AudioClient,
                                      IID_IAUDIORENDERCLIENT,
                                      (void **)&wasapiRenderer->RenderClient);
    if (FAILED(hresult))
    {
        OutputDebugStringA("Failed to acquire render client.\n");
        goto Exit;
    }

    hresult = IAudioClient_GetBufferSize(wasapiRenderer->AudioClient,
                                         &wasapiRenderer->BufferSizeInFrames);
    if (FAILED(hresult))
    {
        OutputDebugStringA("Couldn't obtain total buffer frame count.\n");
        goto Exit;
    }

    DWORD taskIndex = 0;
    wasapiRenderer->Task = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);

    if (wasapiRenderer->Task == 0)
    {
        OutputDebugStringA("AvSetMmThreadCharacteristics\n");
        goto Exit;
    }

    PropVariantClear(&varAudioFormat);
    OutputDebugStringA("WASAPI Initialized!\n");
    return true;

Exit:
    Win32ReleaseWASAPI(wasapiRenderer);

    return false;
}
