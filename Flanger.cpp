/*
  ==============================================================================
  Project: Guitar Pedal Emulation Plug-in
  Author: Jacob Cayetano
  Framework: JUCE
  File: Phaser.h
  Description: Describes phaser circuit, modelled after PhaseShifter object in "Designing Audio Effect Plugins..."
  but modified to contain only four APFs
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
/*
bool Flanger::reset(double _sampleRate, int channel)
{
	return modDelay.reset(_sampleRate, channel);
}

FlangerStruct Flanger::getParameters()
{
	//modDelay.getParameters();
	return flangerStructure;
}

void Flanger::setParameters(const FlangerStruct& params) //Parameters change
{
	if (params.lfoRate != flangerStructure.lfoRate)
	{
		OscillatorParameters lfoParams = modDelay.lfo.getParameters();
		lfoParams.frequency_Hz = params.lfoRate;
		modDelay.lfo.setParameters(lfoParams);
	}
	flangerStructure = params;
}

float Flanger::processAudioSample(float xn, int channel)
{
	return modDelay.processAudioSample(xn, channel);
}

bool Flanger::canProcessAudioFrame()
{
	return modDelay.canProcessAudioFrame();
}

bool Flanger::processAudioFrame(float* inputFrame, float* outputFrame, uint32_t inputChannels, uint32_t outputChannels, int channel)
{
	return modDelay.processAudioFrame(inputFrame,outputFrame,inputChannels,outputChannels,channel);
}*/