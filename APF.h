/*
  ==============================================================================
  Project: Guitar Pedal Emulation Plug-in
  Author: Jacob Cayetano
  Framework: JUCE
  File: APF.h
  Description: Describes all-pass filter for use in phaser circuit
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

class APF : public AudioFilter
{
public:
	APF() 
	{
		// Initialize APF parameters that were declared in APF.h
		audioFilterParameters.algorithm = filterAlgorithm::kAPF1;
		audioFilterParameters.fc = 1000.0; // 20 kHz
		audioFilterParameters.Q = 20; // Quality factor
		audioFilterParameters.boostCut_dB = 0.0;
		coeffArray[c0] = 1.0;
		coeffArray[d0] = 0.0;
	};
	~APF() {};

protected:
	
private:
};