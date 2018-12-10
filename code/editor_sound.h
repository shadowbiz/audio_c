#pragma once

enum AudioEncoding
{
    FLOAT_32,
    FLOAT_24,
    PCM_8,
    PCM_16,
    PCM_24,
    PCM_24_IN_32,
    PCM_32,
    UNSUPPORTED,
};

struct SoundContainer
{
    AudioEncoding Encoding;

    u32 SampleCount;
    u32 SampleRate;
    u32 Channels;

    u8 *Samples;
};

struct SoundOutputBuffer
{
    void (*UpdateFunction)(ThreadContext *, AppMemory *, SoundOutputBuffer *);
};

enum struct SoundError
{
    None,
    NoMem,            // Out of memory.
    InitAudioBackend, // The backend does not appear to be active or running.
    SystemResources,  // A system resource other than memory was not available.
    OpeningDevice,    // Attempted to open a device and failed.
    NoSuchDevice,
    Invalid,            // The programmer did not comply with the API.
    BackendUnavailable, // libsoundio was compiled without support for that backend.

    Streaming, // An open stream had an error that can only be recovered from by destroying the stream and creating it again.

    IncompatibleDevice,
    NoSuchClient, // Attempted to use a device with parameters it cannot support. When JACK returns `JackNoSuchClient`

    IncompatibleBackend, // Attempted to use parameters that the backend cannot support.

    BackendDisconnected, // Backend server shutdown or became inactive.
    Interrupted,

    Underflow, // Buffer underrun occurred.

    EncodingString, // Unable to convert to or from UTF-8 to the native string format.
};

enum struct SoundBackend
{
    //None,
    //Jack,
    //PulseAudio,
    //Alsa,
    CoreAudio,
    Wasapi
};

enum struct SoundFormat
{
    Invalid,
    S8,        // Signed 8 bit
    U8,        // Unsigned 8 bit
    S16LE,     // Signed 16 bit Little Endian
    S16BE,     // Signed 16 bit Big Endian
    U16LE,     // Unsigned 16 bit Little Endian
    U16BE,     // Unsigned 16 bit Little Endian
    S24LE,     // Signed 24 bit Little Endian using low three bytes in 32-bit word
    S24BE,     // Signed 24 bit Big Endian using low three bytes in 32-bit word
    U24LE,     // Unsigned 24 bit Little Endian using low three bytes in 32-bit word
    U24BE,     // Unsigned 24 bit Big Endian using low three bytes in 32-bit word
    S32LE,     // Signed 32 bit Little Endian
    S32BE,     // Signed 32 bit Big Endian
    U32LE,     // Unsigned 32 bit Little Endian
    U32BE,     // Unsigned 32 bit Big Endian
    Float32LE, // Float 32 bit Little Endian, Range -1.0 to 1.0
    Float32BE, // Float 32 bit Big Endian, Range -1.0 to 1.0
    Float64LE, // Float 64 bit Little Endian, Range -1.0 to 1.0
    Float64BE, // Float 64 bit Big Endian, Range -1.0 to 1.0
};

//#define MAX_CHANNELS 24

struct SoundChannelLayout
{
    const char *Name;
    u32 ChannelCount;
    //SoundChannelId Channels[MAX_CHANNELS];
};

struct SoundSampleRateRange
{
    u32 Min;
    u32 Max;
};

enum struct SoundDeviceDirection
{
    Input,
    Output
};

struct SoundIoDevicesInfo
{
    //SoundIoListDevicePtr InputDevices;
    //SoundIoListDevicePtr OutputDevices;
    // can be -1 when default device is unknown
    i32 DefaultOutputIndex;
    i32 DefaultInputIndex;
};

struct SoundEngine
{
    struct SoundDevicesInfo *DevicesInfo;

    void (*OnDevicesChange)(SoundEngine *);
    void (*OnBackendDisconnect)(SoundEngine *, SoundError err);
    void (*OnEventsSignal)(SoundEngine *);
    SoundBackend CurrentBackend;

    void (*EmitRTPriorityWarning)(void);
};

struct SoundDevice
{
    SoundEngine *SoundEngine;

    char *Id;

    char *Name;

    SoundDeviceDirection Direction;
    SoundChannelLayout *Layouts;
    u32 LayoutCount;

    SoundChannelLayout CurrentLayout;

    SoundFormat *Formats;

    u32 FormatCount;
    SoundFormat CurrentFormat;

    SoundSampleRateRange *SampleRates;
    u32 SampleRateCount;
    u32 SampleRateCurrent;
    f64 SoftwareLatencyMin;
    f64 SoftwareLatencyMax;
    f64 SoftwareLatencyCurrent;
    b32 IsRaw;

    u32 DeviceRefCount;

    int ProbeError;
};

struct SoundOutStream
{
    SoundDevice *device;
    SoundFormat format;

    u32 SampleRate;
    SoundChannelLayout layout;
    f64 SoftwareLatency;

    void (*WriteCallback)(SoundOutStream *outStream, u32 frameCountMin, u32 frameCountMax);
    void (*UnderflowCallback)(SoundOutStream *outStream);

    void (*ErrorCallback)(SoundOutStream *outStream, i32 error);

    const char *Name;

    b32 NonTerminalHint;
    u32 BytesPerFrame;
    u32 BytesPerSample;
    i32 LayoutError;
};
