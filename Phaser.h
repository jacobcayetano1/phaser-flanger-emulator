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

#pragma once

#include "fxobjects.h"

struct PhaserStruct {
	PhaserStruct(){}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	PhaserStruct& operator=(const PhaserStruct& pStruct)	// need this override for collections to work
	{
		if (this == &pStruct)
			return *this;

		lfoRate = pStruct.lfoRate;
		lfoDepth = pStruct.lfoDepth;
		intensity = pStruct.intensity;
		quadPhaseLFO = pStruct.quadPhaseLFO;
		phaserFeedbackSwitch = pStruct.phaserFeedbackSwitch;

		return *this;
	}
	// --- individual parameters
	// LFO parameters
	float lfoRate = 1.0f;
	float lfoDepth = 100.0f;
	float intensity = 100.0f;
	bool quadPhaseLFO = false;
	bool phaserFeedbackSwitch = false;
};

class Phaser : public IAudioSignalProcessor
{
public:
	Phaser(void)
	{
		OscillatorParameters lfoParams = lfo.getParameters();
		lfoParams.waveform = generatorWaveform::kLoopingParabola; // kTriangle, kSin, kSaw
		lfo.setParameters(lfoParams);

		AudioFilterParameters filterParams = apf[0].getParameters();
		filterParams.algorithm = filterAlgorithm::kAPF1; // kAPF 1 or 2?
		// params.Q = 0.001; use low Q if using 2nd order APFs

		const int APF_COUNT = 4;

		for (int i = 0; i < APF_COUNT; i++)
		{
			filterParams.fc = 100.0; // set critical frequency
			apf[i].setParameters(filterParams);
		}
	};
	~Phaser(void) {};

	bool reset(double _sampleRate, int channel);

	PhaserStruct getParameters();

	void setParameters(const PhaserStruct& params); //Parameters change

	float processAudioSample(float xn, int channel, double _sampleRate);

	bool canProcessAudioFrame();

protected:
	PhaserStruct phaserStructure;
	APF apf[4]; // 100Hz
	LFO lfo;
private:
	
};