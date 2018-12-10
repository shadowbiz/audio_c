#pragma once

#ifndef CINTERFACE
#define CINTERFACE
#endif

#ifndef COBJMACROS
#define COBJMACROS
#endif

typedef LONGLONG REFERENCE_TIME;

#define IS_EQUAL_GUID(a, b) IsEqualGUID(*(a), *(b))
#define IS_EQUAL_IID(a, b) IsEqualIID((a), *(b))

#define IID_IAUDIOCLIENT (IID_IAudioClient)
#define IID_IMMENDPOINT (IID_IMMEndpoint)
#define IID_IAUDIOCLOCKADJUSTMENT (IID_IAudioClockAdjustment)
#define IID_IAUDIOSESSIONCONTROL (IID_IAudioSessionControl)
#define IID_IAUDIORENDERCLIENT (IID_IAudioRenderClient)
#define IID_IMMDEVICEENUMERATOR (IID_IMMDeviceEnumerator)
#define IID_IAUDIOCAPTURECLIENT (IID_IAudioCaptureClient)
#define CLSID_MMDEVICEENUMERATOR (CLSID_MMDeviceEnumerator)
#define PKEY_DEVICE_FRIENDLYNAME (PKEY_Device_FriendlyName)
#define PKEY_AUDIOENGINE_DEVICEFORMAT (PKEY_AudioEngine_DeviceFormat)

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator =
    {
        0xa95664d2, 0x9614, 0x4f35, {0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6}};

const IID IID_IMMNotificationClient =
    {
        0x7991eec9, 0x7e89, 0x4d85, {0x83, 0x90, 0x6c, 0x70, 0x3c, 0xec, 0x60, 0xc0}};

const IID IID_IAudioClient =
    {
        0x1cb9ad4c, 0xdbfa, 0x4c32, {0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2}};

const IID IID_IAudioRenderClient =
    {
        0xf294acfc, 0x3146, 0x4483, {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}};

const IID IID_IAudioSessionControl =
    {
        0xf4b1a599, 0x7266, 0x4319, {0xa8, 0xca, 0xe7, 0x0a, 0xcb, 0x11, 0xe8, 0xcd}};

const IID IID_IAudioSessionEvents =
    {
        0x24918acc, 0x64b3, 0x37c1, {0x8c, 0xa9, 0x74, 0xa6, 0x6e, 0x99, 0x57, 0xa8}};

const IID IID_IMMEndpoint =
    {
        0x1be09788, 0x6894, 0x4089, {0x85, 0x86, 0x9a, 0x2a, 0x6c, 0x26, 0x5a, 0xc5}};

const IID IID_IAudioClockAdjustment =
    {
        0xf6e4c0a0, 0x46d9, 0x4fb8, {0xbe, 0x21, 0x57, 0xa3, 0xef, 0x2b, 0x62, 0x6c}};

const IID IID_IAudioCaptureClient =
    {
        0xc8adbd64, 0xe71e, 0x48a0, {0xa4, 0xde, 0x18, 0x5c, 0x39, 0x5c, 0xd3, 0x17}};

const static GUID SOUNDIO_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT =
    {
        0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

const static GUID SOUNDIO_KSDATAFORMAT_SUBTYPE_PCM =
    {
        0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

inline REFERENCE_TIME
ReferenceTimeFromMs(f64 ms)
{
    return (REFERENCE_TIME)(ms * 10000.0);
}

inline REFERENCE_TIME
ReferenceTimeFromSec(f64 sec)
{
    return ReferenceTimeFromMs(sec * 1000.0);
}

inline f64
SecondsFromReferenceTime(REFERENCE_TIME time)
{
    return (f64)(time / 10000.0);
}

inline f64
MilliSecondsFromReferenceTime(REFERENCE_TIME time)
{
    return SecondsFromReferenceTime((f64)(time / 1000.0));
}

struct WasapiRenderer
{
    IMMDeviceEnumerator *Enumerator;
    IMMDevice *Device;
    IAudioClient *AudioClient;
    IAudioRenderClient *RenderClient;

    AudioEncoding Encoding;

    u32 Channels;
    u32 SampleRate;

    u32 FrameSizeInBytes;
    u32 BufferSizeInFrames;

    HANDLE Task;
    HANDLE CallbackEvent;

    AppMemory *Memory;
    GetSoundSamplesType *GetSoundSamples;
};
