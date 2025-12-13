#pragma once

#ifndef AUDIO_SOURCE_H
#define AUDIO_SOURCE_H

#include <Ice/Core/Component.h>
#include <AL/al.h>
#include <string>

class AudioClip;

class AudioSource : public Component
{
public:
    AudioSource();
    ~AudioSource();

    void Ready() override;
    void Update() override;

    // Playback control
    void Play();
    void Pause();
    void Stop();
    void Resume();

    bool IsPlaying() const;
    bool IsPaused() const;

    // Clip
    void SetClip(AudioClip* clip);
    void SetClip(const std::string& clipName);
    AudioClip* GetClip() const { return clip; }

    // Properties
    void SetVolume(float volume);
    float GetVolume() const { return volume; }

    void SetPitch(float pitch);
    float GetPitch() const { return pitch; }

    void SetLooping(bool loop);
    bool IsLooping() const { return looping; }

    void SetPlayOnReady(bool play) { playOnReady = play; }
    bool GetPlayOnReady() const { return playOnReady; }

    // 3D audio settings
    void SetSpatial(bool spatial);
    bool IsSpatial() const { return spatial; }
    
    void SetMinDistance(float distance);
    float GetMinDistance() const { return minDistance; }

    void SetMaxDistance(float distance);
    float GetMaxDistance() const { return maxDistance; }

    // Playback position
    void SetPlaybackTime(float seconds);
    float GetPlaybackTime() const;

private:
    ALuint source;
    AudioClip* clip;

    float volume;
    float pitch;
    bool looping;
    bool playOnReady;

    bool spatial;
    float minDistance;
    float maxDistance;

    void UpdatePosition();
};

#endif