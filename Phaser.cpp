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

#include "Phaser.h"

bool Phaser::reset(double _sampleRate, int channel)
{
	lfo.reset(_sampleRate, channel);

	for (int i = 0; i < 4; i++)
	{
		apf[i].reset(_sampleRate, channel);
	}

	return true;
}

PhaserStruct Phaser::getParameters() { return phaserStructure; }

void Phaser::setParameters(const PhaserStruct& params) //Parameters change
{
	if (params.lfoRate != phaserStructure.lfoRate)
	{
		OscillatorParameters lfoParams = lfo.getParameters();
		lfoParams.frequency_Hz = params.lfoRate;
		lfo.setParameters(lfoParams);
	}
	phaserStructure = params;
}

float Phaser::processAudioSample(float xn, int channel, double _sampleRate)
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

	float Sn;
	// Create combined feedback
	if (phaserStructure.phaserFeedbackSwitch)
	{
		Sn = gamma3 * apf[0].getS_value(channel) +
			gamma2 * apf[1].getS_value(channel);
	}
	else
	{
		Sn = gamma3 * apf[0].getS_value(channel) +
			gamma2 * apf[1].getS_value(channel) +
			gamma1 * apf[2].getS_value(channel) +
			apf[3].getS_value(channel);
	}

	// Form input to first APF
	float u = alpha0 * (xn + K * Sn);

	// Cascade of APFs
	float apf0_out = apf[0].processAudioSample(u, channel, _sampleRate);
	float apf1_out = apf[1].processAudioSample(apf0_out, channel, _sampleRate);
	float apf2_out = apf[2].processAudioSample(apf1_out, channel, _sampleRate);
	float apf3_out = apf[3].processAudioSample(apf2_out, channel, _sampleRate);

	// Sum with national semiconductor design ratio
	// return 0.5*xn + 5.0 * apf3_out;
	// return 0.25*xn + 2.5 * apf3_out;
	//return 0.125 * xn + 1.25 * apf3_out;
	//return apf3_out;
	return 0.707 * xn + 0.707 * apf3_out;
}

bool Phaser::canProcessAudioFrame() { return false; }