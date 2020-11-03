/*
  ==============================================================================
  Project: Guitar Pedal Emulation Plug-in
  Author: Jacob Cayetano
  Framework: JUCE
  File: Flanger.cpp
  Description: Flanger DSP
  Contains Code From:
  --- TheAudioProgrammer
  --- ASPIK Code Library
  --- Designing Audio Effect Plugins in C++ by Will C. Pirkle
  References:
  --- TheAudioProgrammer (YouTube & GitHub)
  --- JUCE Framework Documentation
  --- ASPIK Code Documentation
  --- Designing Audio Effect Plugins in C++ for AAX, AU, and VST3 with DSP Theory by Will C. Pirkle
  --- C++: From Control Structures Through Objects (9th Edition) by Tony Gaddis
  --- Getting Started with JUCE by Martin Robinson
  ==============================================================================
*/

#include "Flanger.h"
#define _USE_MATH_DEFINES
#include <math.h>

bool Flanger::reset(double sampleRate, int inputChannels)
{
    float maxDelayTime = 0.02f + 0.02f;
    delayBufferSamples = (int)(maxDelayTime * (float)sampleRate) + 1;
    if (delayBufferSamples < 1)
    {
        delayBufferSamples = 1;
    }

    delayBuffer.setSize(inputChannels, delayBufferSamples);
    delayBuffer.clear();

    delayWritePosition = 0;
    lfoPhase = 0.0f;
    inverseSampleRate = 1.0f / (float)sampleRate;
    twoPi = 2.0f * M_PI;

    return true;
}

float Flanger::lfo(float phase, int waveform)
{
    float out = 0.0f;

    switch (waveform) 
    {
        case waveformSine: 
        {
            out = 0.5f + 0.5f * sinf(twoPi * phase);
            break;
        }
        case waveformTriangle:
        {
            if (phase < 0.25f)
                out = 0.5f + 2.0f * phase;
            else if (phase < 0.75f)
                out = 1.0f - 2.0f * (phase - 0.25f);
            else
                out = 2.0f * (phase - 0.75f);
            break;
        }
        case waveformSawtooth: 
        {
            if (phase < 0.5f)
                out = 0.5f + phase;
            else
                out = phase - 0.5f;
            break;
        }
        case waveformInverseSawtooth: 
        {
            if (phase < 0.5f)
                out = 0.5f - phase;
            else
                out = 1.5f - phase;
            break;
        }
    }

    return out;
}

float Flanger::processAudioSample(float xn, int* localWritePosition, float* phase, double sampleRate)
{
    const float in = xn;
    float out = 0.0f;

    float localDelayTime = (0.0025f + 0.001f * lfo(*phase, 1)) * (float)sampleRate;

    float readPosition = fmodf((float)*localWritePosition - localDelayTime + (float)delayBufferSamples, delayBufferSamples);
    int localReadPosition = floorf(readPosition);

    // Cubic Interpolation
    float fraction = readPosition - (float)localReadPosition;
    float fractionSqrt = fraction * fraction;
    float fractionCube = fractionSqrt * fraction;
    
    float sample0 = delayData[(localReadPosition - 1 + delayBufferSamples) % delayBufferSamples];
    float sample1 = delayData[(localReadPosition + 0)];
    float sample2 = delayData[(localReadPosition + 1) % delayBufferSamples];
    float sample3 = delayData[(localReadPosition + 2) % delayBufferSamples];

    float a0 = -0.5f * sample0 + 1.5f * sample1 - 1.5f * sample2 + 0.5f * sample3;
    float a1 = sample0 - 2.5f * sample1 + 2.0f * sample2 - 0.5f * sample3;
    float a2 = -0.5f * sample0 + 0.5f * sample2;
    float a3 = sample1;
    out = a0 * fractionCube + a1 * fractionSqrt + a2 * fraction + a3;

    //channelData[sample] = in + out * (*treeState.getRawParameterValue(FLANGER_DEPTH_ID) /100.0f); //currentInverted;
    float output = in + out * 1.0f * 1.0f;
    delayData[*localWritePosition] = in + out * 0.5f; //* 0.5f;//currentFeedback;

    if (++*localWritePosition >= delayBufferSamples)
        *localWritePosition -= delayBufferSamples;

    *phase += 5.0f * inverseSampleRate;
    if (*phase >= 1.0f)
    {
        *phase -= 1.0f;
    }
    return output;
}