#include <Ice/Resources/AudioClip.h>

#include <fstream>
#include <vector>
#include <cstring>
#include <iostream>

AudioClip::AudioClip()
    : buffer(0)
    , monoBuffer(0)
    , duration(0.0f)
    , sampleRate(0)
    , channels(0)
{
}

AudioClip::~AudioClip()
{
    Unload();
}

bool AudioClip::LoadFromFile(const std::string& filepath)
{
    Unload();

    if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".wav")
    {
        return LoadWAV(filepath);
    }

    return false;
}

void AudioClip::Unload()
{
    if (buffer != 0)
    {
        alDeleteBuffers(1, &buffer);
        buffer = 0;
    }
    if (monoBuffer != 0)
    {
        alDeleteBuffers(1, &monoBuffer);
        monoBuffer = 0;
    }
    duration = 0.0f;
    sampleRate = 0;
    channels = 0;
}

ALuint AudioClip::GetMonoBuffer()
{
    if (channels == 1)
    {
        return buffer;
    }
    
    if (monoBuffer == 0)
    {
        CreateMonoBuffer();
    }
    
    return monoBuffer;
}

void AudioClip::CreateMonoBuffer()
{
    if (buffer == 0 || channels != 2 || rawData.empty())
    {
        return;
    }
    
    std::vector<char> monoData;
    
    if (bitsPerSample == 16)
    {
        int16_t* stereo = reinterpret_cast<int16_t*>(rawData.data());
        size_t sampleCount = rawData.size() / 4; // 2 channels * 2 bytes
        monoData.resize(sampleCount * 2);
        int16_t* mono = reinterpret_cast<int16_t*>(monoData.data());
        
        for (size_t i = 0; i < sampleCount; i++)
        {
            int32_t mixed = (static_cast<int32_t>(stereo[i * 2]) + static_cast<int32_t>(stereo[i * 2 + 1])) / 2;
            mono[i] = static_cast<int16_t>(mixed);
        }
    }
    else // 8-bit
    {
        size_t sampleCount = rawData.size() / 2; // 2 channels * 1 byte
        monoData.resize(sampleCount);
        
        for (size_t i = 0; i < sampleCount; i++)
        {
            int16_t mixed = (static_cast<int16_t>(rawData[i * 2]) + static_cast<int16_t>(rawData[i * 2 + 1])) / 2;
            monoData[i] = static_cast<char>(mixed);
        }
    }
    
    ALenum monoFormat = (bitsPerSample == 16) ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
    
    alGenBuffers(1, &monoBuffer);
    alBufferData(monoBuffer, monoFormat, monoData.data(), monoData.size(), sampleRate);
}

bool AudioClip::LoadWAV(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Failed to open file " << filepath << std::endl;
        return false;
    }

    // RIFF header
    char riff[4];
    file.read(riff, 4);
    if (strncmp(riff, "RIFF", 4) != 0)
    {
        std::cout << "File does not appear to be a valid WAV file!" << std::endl;
        return false;
    }

    file.seekg(4, std::ios::cur);

    // WAVE header
    char wave[4];
    file.read(wave, 4);
    if (strncmp(wave, "WAVE", 4) != 0)
    {
        std::cout << "File does not appear to be a valid WAV file!" << std::endl;
        return false;
    }

    // Find fmt chunk
    char chunkId[4];
    uint32_t chunkSize;

    while (file.read(chunkId, 4))
    {
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (strncmp(chunkId, "fmt ", 4) == 0)
        {
            uint16_t audioFormat;
            uint16_t numChannels;
            uint32_t sampleRateVal;
            uint32_t byteRate;
            uint16_t blockAlign;

            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRateVal), 4);
            file.read(reinterpret_cast<char*>(&byteRate), 4);
            file.read(reinterpret_cast<char*>(&blockAlign), 2);
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            // Skip any extra format bytes
            if (chunkSize > 16)
            {
                file.seekg(chunkSize - 16, std::ios::cur);
            }

            channels = numChannels;
            sampleRate = sampleRateVal;

            // Determine OpenAL format
            ALenum format;
            if (numChannels == 1)
            {
                format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
            }
            else
            {
                format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
            }

            // Find data chunk
            while (file.read(chunkId, 4))
            {
                file.read(reinterpret_cast<char*>(&chunkSize), 4);

                if (std::strncmp(chunkId, "data", 4) == 0)
                {
                    std::vector<char> data(chunkSize);
                    file.read(data.data(), chunkSize);

                    // Store for potential mono conversion
                    rawData = data;
                    
                    // Calculate duration
                    duration = static_cast<float>(chunkSize) / (sampleRateVal * numChannels * (bitsPerSample / 8));

                    // Create OpenAL buffer
                    alGenBuffers(1, &buffer);
                    alBufferData(buffer, format, data.data(), chunkSize, sampleRateVal);

                    ALenum error = alGetError();
                    if (error != AL_NO_ERROR)
                    {
                        std::cout << "OpenAL error: " << error << std::endl;
                        std::cout << "Format: " << format << " Channels: " << numChannels << " BitsPerSample: " << bitsPerSample << " SampleRate: " << sampleRateVal << " DataSize: " << chunkSize << std::endl;
                        alDeleteBuffers(1, &buffer);
                        buffer = 0;
                        return false;
                    }

                    return true;
                }
                else
                {
                    file.seekg(chunkSize, std::ios::cur);
                }
            }
        }
        else
        {
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    return false;
}