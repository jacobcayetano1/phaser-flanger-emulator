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

#include "APF.h"

struct PhaserStruct {
	PhaserStruct(){}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	PhaserStruct& operator=(const PhaserStruct& pStruct)	// need this override for collections to work
	{
		if (this == &pStruct)
			return *this;
		/*for (int i = 0; i < 4; i++)
		{
			apf[i] = pStruct.apf[i];
		}*/
		lfoRate = pStruct.lfoRate;
		lfoDepth = pStruct.lfoDepth;
		intensity = pStruct.intensity;
		quadPhaseLFO = pStruct.quadPhaseLFO;
		//lfo = pStruct.lfo;

		return *this;
	}
	// --- individual parameters
	//APF apf[4]; // 100Hz
	//LFO lfo;
	// LFO parameters
	float lfoRate = 1000.0f;
	float lfoDepth = 100.0f;
	float intensity = 75.0f;
	bool quadPhaseLFO = false;
};

class Phaser : public IAudioSignalProcessor
{
public:
	Phaser(void)
	{
		OscillatorParameters lfoParams = lfo.getParameters();
		lfoParams.waveform = generatorWaveform::kSin; // sine LFO
		//lfoParams.frequency_Hz = 0.0;
		lfo.setParameters(lfoParams);

		AudioFilterParameters filterParams = apf[0].getParameters();
		filterParams.algorithm = filterAlgorithm::kAPF1; // kAPF 1 or 2?
		// params.Q = 0.001; use low Q if using 2nd order APFs

		for (int i = 0; i < 4; i++)
		{
			switch(i)
			{
			case 0:
				filterParams.fc = 100.0; // set critical frequency
				apf[i].setParameters(filterParams);
				break;
			case 1:
				filterParams.fc = 200.0; // set critical frequency
				apf[i].setParameters(filterParams);
				break;
			case 2:
				filterParams.fc = 400.0; // set critical frequency
				apf[i].setParameters(filterParams);
				break;
			case 3:
				filterParams.fc = 800.0; // set critical frequency
				apf[i].setParameters(filterParams);
				break;
			}
		}
	};
	~Phaser(void) {};

	bool reset(double _sampleRate, int channel) 
	{
		lfo.reset(_sampleRate, channel);
		
		for (int i = 0; i < 4; i++)
		{
			apf[i].reset(_sampleRate, channel);
		}
		
		return true;
	}

	PhaserStruct getParameters() { return phaserStructure; }

	void setParameters(const PhaserStruct& params) //Parameters change
	{
		if (params.lfoRate != phaserStructure.lfoRate)
		{
			OscillatorParameters lfoParams = lfo.getParameters();
			lfoParams.frequency_Hz = params.lfoRate;
			lfo.setParameters(lfoParams);
		}
		phaserStructure = params;
	}

	float processAudioSample(float xn, int channel)
	{
		// SHOW ALGORITHM
		
		SignalGenData lfoDat = lfo.renderAudioOutput();

		// Create bipolar modulator value
		float lfoVal = lfoDat.normalOutput;
		if (phaserStructure.quadPhaseLFO)
		{
			lfoVal = lfoDat.quadPhaseOutput_pos;
		}

		float depth = phaserStructure.lfoDepth / 100.0;
		float modValue = lfoVal * depth;

		// Calculate modulated values for each APF
		for (int i = 0; i < 4; i++)
		{
			AudioFilterParameters filterParams = apf[i].getParameters();
			// May change values in future, may change to reference different constants
			switch (i)
			{
			case 0:
				filterParams.fc = doBipolarModulation(modValue, apf0_minF, apf0_maxF);
				break;
			case 1:
				filterParams.fc = doBipolarModulation(modValue, apf1_minF, apf1_maxF);
				break;
			case 2:
				filterParams.fc = doBipolarModulation(modValue, apf2_minF, apf2_maxF);
				break;
			case 3:
				filterParams.fc = doBipolarModulation(modValue, apf3_minF, apf3_maxF);
				break;
			}
			apf[i].setParameters(filterParams);
		}

		// Calculate gamma values
		float gamma1 = apf[3].getG_value();
		float gamma2 = apf[2].getG_value() * gamma1;
		float gamma3 = apf[1].getG_value() * gamma2;
		float gamma4 = apf[0].getG_value() * gamma3;

		// Set alpha values
		float K = phaserStructure.intensity / 100.0;
		float alpha0 = 1.0 / (1.0 + K * gamma4);

		// Create combined feedback
		float Sn = gamma3 * apf[0].getS_value(channel) +
			gamma2 * apf[1].getS_value(channel) +
			gamma1 * apf[2].getS_value(channel) +
			apf[3].getS_value(channel);

		// Form input to first APF
		float u = alpha0 * (xn + K * Sn); // + or - ?

		// Cascade of APFs
		float apf0_out = apf[0].processAudioSample(u, channel);
		float apf1_out = apf[1].processAudioSample(apf0_out, channel);
		float apf2_out = apf[2].processAudioSample(apf1_out, channel);
		float apf3_out = apf[3].processAudioSample(apf2_out, channel);

		// Sum with -3db coeffs
		//return 0.707 * xn + 0.707 * apf3_out;
		// Sum with national semiconductor design ratio
		// return 0.5*xn + 5.0 * apf3_out;
		// return 0.25*xn + 2.5 * apf3_out;
		return 0.125 * xn + 1.25 * apf3_out;
	}

	bool canProcessAudioFrame() { return false; }

protected:
	PhaserStruct phaserStructure;
	APF apf[4]; // 100Hz
	LFO lfo;
private:
	
};