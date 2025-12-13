#pragma once

#ifndef AUDIO_CLIP_H
#define AUDIO_CLIP_H

#include <string>
#include <AL/al.h>
#include <vector>

class AudioClip
{
public:
    AudioClip();
    ~AudioClip();

    bool LoadFromFile(const std::string& filepath);
    void Unload();

    ALuint GetBuffer() const {return buffer;}
    ALuint GetMonoBuffer();
    bool IsLoaded() const {return buffer != 0;}

    float GetDuration() const {return duration;}
    float GetSampleRate() const {return sampleRate;}
    float GetChannelCount() const {return channels;}
private:
    ALuint buffer;
    ALuint monoBuffer;
    float duration;
    int sampleRate;
    int channels;
    int bitsPerSample;
    std::vector<char> rawData;

    void CreateMonoBuffer();

    bool LoadWAV(const std::string& filepath);
};

#endif