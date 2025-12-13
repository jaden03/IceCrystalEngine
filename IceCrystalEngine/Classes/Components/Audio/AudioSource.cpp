#include <iostream>
#include <Ice/Components/Audio/AudioSource.h>
#include <Ice/Resources/AudioClip.h>
#include <Ice/Managers/AudioManager.h>
#include <Ice/Core/Actor.h>
#include <Ice/Core/Transform.h>

AudioSource::AudioSource()
    : source(0)
    , clip(nullptr)
    , volume(1.0f)
    , pitch(1.0f)
    , looping(false)
    , playOnReady(false)
    , spatial(true)
    , minDistance(1.0f)
    , maxDistance(100.0f)
{
    alGenSources(1, &source);

    alSourcef(source, AL_GAIN, volume);
    alSourcef(source, AL_PITCH, pitch);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcef(source, AL_REFERENCE_DISTANCE, minDistance);
    alSourcef(source, AL_MAX_DISTANCE, maxDistance);
    alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);
}

AudioSource::~AudioSource()
{
    if (source != 0)
    {
        alSourceStop(source);
        alDeleteSources(1, &source);
        source = 0;
    }
}

void AudioSource::Ready()
{
    if (playOnReady && clip != nullptr)
    {
        Play();
    }
}

void AudioSource::Update()
{
    UpdatePosition();
}

void AudioSource::Play()
{
    if (clip == nullptr || !clip->IsLoaded())
    {
        return;
    }

    ALuint buf = spatial ? clip->GetMonoBuffer() : clip->GetBuffer();
    alSourcei(source, AL_BUFFER, buf);
    UpdatePosition();
    alSourcePlay(source);
}

void AudioSource::Pause()
{
    alSourcePause(source);
}

void AudioSource::Stop()
{
    alSourceStop(source);
}

void AudioSource::Resume()
{
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED)
    {
        alSourcePlay(source);
    }
}

bool AudioSource::IsPlaying() const
{
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool AudioSource::IsPaused() const
{
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

void AudioSource::SetClip(AudioClip* clip)
{
    Stop();
    this->clip = clip;
}

void AudioSource::SetClip(const std::string& clipName)
{
    SetClip(AudioManager::GetInstance().GetClip(clipName));
}

void AudioSource::SetVolume(float volume)
{
    this->volume = glm::clamp(volume, 0.0f, 1.0f);
    alSourcef(source, AL_GAIN, this->volume);
}

void AudioSource::SetPitch(float pitch)
{
    this->pitch = glm::clamp(pitch, 0.1f, 3.0f);
    alSourcef(source, AL_PITCH, this->pitch);
}

void AudioSource::SetLooping(bool loop)
{
    looping = loop;
    alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void AudioSource::SetSpatial(bool spatial)
{
    this->spatial = spatial;
    
    if (spatial)
    {
        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
        UpdatePosition();
    }
    else
    {
        // Non-spatial: position at listener, no distance attenuation
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    }
}

void AudioSource::SetMinDistance(float distance)
{
    minDistance = distance;
    alSourcef(source, AL_REFERENCE_DISTANCE, minDistance);
}

void AudioSource::SetMaxDistance(float distance)
{
    maxDistance = distance;
    alSourcef(source, AL_MAX_DISTANCE, maxDistance);
}

void AudioSource::SetPlaybackTime(float seconds)
{
    alSourcef(source, AL_SEC_OFFSET, seconds);
}

float AudioSource::GetPlaybackTime() const
{
    float seconds = 0.0f;
    alGetSourcef(source, AL_SEC_OFFSET, &seconds);
    return seconds;
}

void AudioSource::UpdatePosition()
{
    if (!spatial || owner == nullptr)
    {
        return;
    }
    
    glm::vec3 pos = transform->position;
    alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
    
    // Get listener position and calculate distance
    float listenerPos[3];
    alGetListener3f(AL_POSITION, &listenerPos[0], &listenerPos[1], &listenerPos[2]);
    glm::vec3 listener(listenerPos[0], listenerPos[1], listenerPos[2]);
    float distance = glm::length(pos - listener);
    if (distance > maxDistance)
    {
        alSourcef(source, AL_GAIN, 0.0f);
    }
    else if (distance > minDistance)
    {
        float t = (distance - minDistance) / (maxDistance - minDistance);
        alSourcef(source, AL_GAIN, volume * (1.0f - t));
    }
    else
    {
        alSourcef(source, AL_GAIN, volume);
    }
}