#include <Ice/Managers/AudioManager.h>
#include <Ice/Resources/AudioClip.h>
#include <Ice/Managers/SceneManager.h>
#include <Ice/Components/Camera.h>
#include <iostream>

AudioManager::AudioManager()
    : device(nullptr)
    , context(nullptr)
    , initialized(false)
    , masterVolume(1.0f)
    , nextOneShotSource(0)
{
    for (int i = 0; i < MAX_ONESHOT_SOURCES; i++)
    {
        oneShotSources[i] = 0;
    }
}

AudioManager::~AudioManager()
{
    Shutdown();
}

AudioManager& AudioManager::GetInstance()
{
    static AudioManager instance;
    return instance;
}

bool AudioManager::Initialize()
{
    if (initialized) return true;

    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
    
    // open default device
    device = alcOpenDevice(nullptr);
    if (!device)
    {
        std::cout << "Failed to open audio device." << std::endl;
        return false;
    }

    // create context
    context = alcCreateContext(device, nullptr);
    if (!context)
    {
        std::cout << "Failed to create audio context." << std::endl;
        alcCloseDevice(device);
        device = nullptr;
        return false;
    }

    if (!alcMakeContextCurrent(context))
    {
        std::cout << "Failed to make audio context current." << std::endl;
        alcDestroyContext(context);
        alcCloseDevice(device);
        context = nullptr;
        device = nullptr;
        return false;
    }

    // Generate one-shot sources
    alGenSources(MAX_ONESHOT_SOURCES, oneShotSources);

    // defualt listener properties
    SetListenerPosition(glm::vec3(0, 0, 0));
    SetListenerVelocity(glm::vec3(0, 0, 0));
    SetListenerOrientation(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    initialized = true;
    std::cout << "AudioManager initialized." << std::endl;
    return true;
}

void AudioManager::Shutdown()
{
    if (!initialized)
        return;

    UnloadAllClips();

    alDeleteSources(MAX_ONESHOT_SOURCES, oneShotSources);

    alcMakeContextCurrent(nullptr);

    if (context)
    {
        alcDestroyContext(context);
        context = nullptr;
    }

    if (device)
    {
        alcCloseDevice(device);
        device = nullptr;
    }

    initialized = false;
}

void AudioManager::Update()
{
    if (!initialized) return;

    Camera* mainCamera = SceneManager::GetInstance().mainCamera;
    if (mainCamera)
    {
        Actor* cameraActor = mainCamera->owner;
        if (cameraActor)
        {
            Transform* transform = cameraActor->transform;
            SetListenerPosition(transform->position);
            SetListenerOrientation(transform->forward, transform->up);
            glm::vec3 listenerPos = transform->position;
        }
    }
}


void AudioManager::SetListenerPosition(const glm::vec3& position)
{
    alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void AudioManager::SetListenerVelocity(const glm::vec3& velocity)
{
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}


void AudioManager::SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up)
{
    float orientation[6] = {
        forward.x, forward.y, forward.z,
        up.x, up.y, up.z
    };
    alListenerfv(AL_ORIENTATION, orientation);
}

void AudioManager::SetMasterVolume(float volume)
{
    masterVolume = glm::clamp(volume, 0.0f, 1.0f);
    alListenerf(AL_GAIN, masterVolume);
}


AudioClip* AudioManager::LoadClip(const std::string& name, const std::string& filepath)
{
    // Check if already loaded
    auto it = clips.find(name);
    if (it != clips.end())
    {
        return it->second.get();
    }

    auto clip = std::make_unique<AudioClip>();
    if (!clip->LoadFromFile(filepath))
    {
        std::cerr << "AudioManager: Failed to load clip: " << filepath << std::endl;
        return nullptr;
    }

    AudioClip* ptr = clip.get();
    clips[name] = std::move(clip);
    return ptr;
}


AudioClip* AudioManager::GetClip(const std::string& name)
{
    auto it = clips.find(name);
    if (it != clips.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void AudioManager::UnloadClip(const std::string& name)
{
    clips.erase(name);
}

void AudioManager::UnloadAllClips()
{
    clips.clear();
}

void AudioManager::PlayOneShot(const std::string& clipName, const glm::vec3& position, float volume, float pitch)
{
    AudioClip* clip = GetClip(clipName);
    if (!clip || !clip->IsLoaded())
    {
        return;
    }

    ALuint source = GetAvailableOneShotSource();

    alSourcei(source, AL_BUFFER, clip->GetBuffer());
    alSource3f(source, AL_POSITION, position.x, position.y, position.z);
    alSourcef(source, AL_GAIN, volume);
    alSourcef(source, AL_PITCH, pitch);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    alSourcePlay(source);
}

ALuint AudioManager::GetAvailableOneShotSource()
{
    // Find a source that's not playing
    for (int i = 0; i < MAX_ONESHOT_SOURCES; i++)
    {
        int index = (nextOneShotSource + i) % MAX_ONESHOT_SOURCES;
        ALint state;
        alGetSourcei(oneShotSources[index], AL_SOURCE_STATE, &state);

        if (state != AL_PLAYING)
        {
            nextOneShotSource = (index + 1) % MAX_ONESHOT_SOURCES;
            return oneShotSources[index];
        }
    }

    // All sources busy, reuse the next one anyway
    ALuint source = oneShotSources[nextOneShotSource];
    alSourceStop(source);
    nextOneShotSource = (nextOneShotSource + 1) % MAX_ONESHOT_SOURCES;
    return source;
}