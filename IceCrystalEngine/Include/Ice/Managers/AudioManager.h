#pragma once

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <memory>

#include "Ice/Resources/AudioClip.h"

#pragma comment(lib, "OpenAL32.lib")

class AudioManager
{
public:
    static AudioManager& GetInstance();

    bool Initialize();
    void Shutdown();

    void Update();

    // Listener (follows main camera)
    void SetListenerPosition(const glm::vec3& position);
    void SetListenerVelocity(const glm::vec3& velocity);
    void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);

    // Global volume
    void SetMasterVolume(float volume);
    float GetMasterVolume() const { return masterVolume; }

    // Clip management
    AudioClip* LoadClip(const std::string& name, const std::string& path);
    AudioClip* GetClip(const std::string& name);
    void UnloadClip(const std::string& name);
    void UnloadAllClips();

    // One shot sounds without needing an audiosource
    void PlayOneShot(const std::string& name, const glm::vec3& position, float volume = 1.0f, float pitch = 1.0f);

    bool IsInitialized() const { return initialized; }
private:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    void operator=(const AudioManager&) = delete;

    ALCdevice* device;
    ALCcontext* context;
    bool initialized;
    float masterVolume;

    std::unordered_map<std::string, std::unique_ptr<AudioClip>> clips;

    // One-shot source pool
    static const int MAX_ONESHOT_SOURCES = 32;
    ALuint oneShotSources[MAX_ONESHOT_SOURCES];
    int nextOneShotSource;

    ALuint GetAvailableOneShotSource();
};

#endif