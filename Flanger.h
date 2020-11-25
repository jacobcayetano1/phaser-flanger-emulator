/*
  ==============================================================================
  Project: Guitar Pedal Emulation Plug-in
  Author: Jacob Cayetano
  Framework: JUCE
  File: Flanger.h
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

#pragma once

//#include "fxobjects.h"
#include <JuceHeader.h>

class Flanger
{
public:
	Flanger(void)
	{

	};
	~Flanger(void) {};

	bool reset(double sampleRate, int inputChannels);

	float lfo(float phase, int waveform);

	float processAudioSample(float xn, int* localWritePosition, float* phase, double sampleRate);
    
    enum waveformIndex {
        waveformSine = 0,
        waveformTriangle,
        waveformSawtooth,
        waveformInverseSawtooth,
    };

    AudioSampleBuffer delayBuffer;
	float* delayData;
    int delayBufferSamples;
    //int delayBufferChannels;
    int delayWritePosition;

    float lfoPhase;
    float inverseSampleRate;
    float twoPi;
protected:
private:
};

/*
class Flanger : public ModulatedDelay
{
public:
	Flanger(void)
	{
		OscillatorParameters lfoParams = lfo.getParameters();
		lfoParams.waveform = generatorWaveform::kTriangle; // kTriangle, kSin, kSaw
		lfo.setParameters(lfoParams);
		
		parameters.algorithm = modDelaylgorithm::kFlanger;
		//parameters.lfoRate_Hz = 10.0f;
		//parameters.lfoDepth_Pct = 100.0f;
		parameters.feedback_Pct = 75.0f;
	};
	~Flanger(void) {};
	// maybe remove flanger.h?
protected:
private:
};*/