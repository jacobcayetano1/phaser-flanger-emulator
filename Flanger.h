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

#include "fxobjects.h"

struct FlangerStruct {
	FlangerStruct() {}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	FlangerStruct& operator=(const FlangerStruct& fStruct)	// need this override for collections to work
	{
		if (this == &fStruct)
			return *this;
		algorithm = fStruct.algorithm;
		lfoRate = fStruct.lfoRate;
		lfoDepth = fStruct.lfoDepth;
		feedback = fStruct.feedback;
		drywet = fStruct.drywet;

		return *this;
	}
	// --- individual parameters
	// LFO parameters
	modDelaylgorithm algorithm = modDelaylgorithm::kFlanger;
	float lfoRate = 1000.0f;
	float lfoDepth = 100.0f;
	float feedback = 75.0f;

	float drywet = 50.0f;
};

class Flanger : public IAudioSignalProcessor
{
public:
	Flanger(void)
	{
		
	};
	~Flanger(void) {};

	bool reset(double _sampleRate, int channel);

	FlangerStruct getParameters();

	void setParameters(const FlangerStruct& params); //Parameters change

	float processAudioSample(float xn, int channel);

	bool canProcessAudioFrame();

protected:
	FlangerStruct flangerStructure;
	ModulatedDelay modDelay;
private:
};