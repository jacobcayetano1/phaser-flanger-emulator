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
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
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

    flangerLowShelf.reset(sampleRate, 0);
    flangerLowShelf.reset(sampleRate, 1);

    // Initializes low shelf of flanger
    AudioFilterParameters lowShelfParams = flangerLowShelf.getParameters();
    lowShelfParams.algorithm = filterAlgorithm::kLowShelf;
    lowShelfParams.boostCut_dB = -6.0f;
    lowShelfParams.fc = 150.0f;
    flangerLowShelf.setParameters(lowShelfParams);

    return true;
}

void Flanger::setParameters(float flangerDepth, float flangerRate, float flangerFeedback, bool flangerInverted)
{
    flangerDepth_internal = flangerDepth;
    flangerRate_internal = flangerRate;
    flangerFeedback_internal = flangerFeedback;
    if (flangerInverted)
    {
        flangerInverted_internal = -1.0f;
    }
    else
    {
        flangerInverted_internal = 1.0f;
    }
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

float Flanger::processAudioSample(float xn, int* writePos, float* phase, double sampleRate, int channel)
{
    const float in = xn;
    float out = 0.0f;

    // local delay = (set delay + set width * (LFO value between -1,1)) * sample rate -> to get local delay in samples
    float delayInSamples = (0.0075f + 0.001f * lfo(*phase, 1)) * (float)sampleRate;
    
    ///////////////////////// Cubic Interpolation //////////////////////////////////////////
    
    // Obtain position at which to read delay line
    float readPos = fmodf((float)*writePos - delayInSamples + (float)delayBufferSamples, delayBufferSamples);
    // Obtain whole number component of read position
    int intRead = floorf(readPos);
    // Obtain fractional component of read position
    float fractionRead = readPos - (float)intRead;
    float fractionSq = fractionRead * fractionRead;
    float fractionCube = fractionSq * fractionRead;
    
    // Obtain four samples associated with read position
    float sample0 = delayData[(intRead - 1 + delayBufferSamples) % delayBufferSamples];
    float sample1 = delayData[(intRead + 0)];
    float sample2 = delayData[(intRead + 1) % delayBufferSamples];
    float sample3 = delayData[(intRead + 2) % delayBufferSamples];

    // Calculate output of delay line based on cubic interpolation formula
    float a0 = -0.5f * sample0 + 1.5f * sample1 - 1.5f * sample2 + 0.5f * sample3;
    float a1 = sample0 - 2.5f * sample1 + 2.0f * sample2 - 0.5f * sample3;
    float a2 = -0.5f * sample0 + 0.5f * sample2;
    float a3 = sample1;
    out = a0 * fractionCube + a1 * fractionSq + a2 * fractionRead + a3;

    ////////////////////////////////////////////////////////////////////////////////////////

    float output = in + out * flangerDepth_internal * flangerInverted_internal;
    delayData[*writePos] = in + out * flangerFeedback_internal;

    if (++*writePos >= delayBufferSamples)
        *writePos -= delayBufferSamples;

    *phase += flangerRate_internal * inverseSampleRate;
    if (*phase >= 1.0f)
    {
        *phase -= 1.0f;
    }
    
    //if (flangerFeedback_internal < 0.0f)
    //{
         output = flangerLowShelf.processAudioSample(output, channel, sampleRate);
    //}

    return output;
}