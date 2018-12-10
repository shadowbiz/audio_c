#include "editor_sound.h"

#if 0
global_variable const SoundBackend AvailableBackends[] =
{
    SoundBackend::CoreAudio,
    SoundBackend::Wasapi
};

typedef SoundError (*BackendInitType)();
global_variable BackendInitType BackendInitFunctions[] =
{
    0, // None backend
    CoreAudioInit,
    WasapiInit
};

SoundError
SoundConnect()
{
    auto error = SoundError::None;
    
    for (int i = 0; i != ArrayCount(AvailableBackends); ++i)
    {
        SoundBackend backend = AvailableBackends[i];
        error = ConnectBackend(backend);
        if (error != SoundError::None)
        {
            return error;
        }
        
        if (error != SoundError::InitAudioBackend)
        {
            return error;
        }
    }
    return error;
    
}

SoundError
ConnectBackend(SoundBackend backend)
{
    auto error = SoundError::None;
    
    error = BackendInitFunctions[(int)backend]();
    
    if (error != SoundError::None)
    {
        SoundDisconnect();
    }
    
    return error;
}
#endif