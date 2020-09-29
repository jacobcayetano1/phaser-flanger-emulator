/*
  ==============================================================================
  Project: Guitar Pedal Emulation Plug-in
  Author: Jacob Cayetano
  Framework: JUCE
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

#include <JuceHeader.h>
#include "Phaser.h"
//#include "APF.h"
#include <string>

//==============================================================================
/**
*/
class PedalEmulatorAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    PedalEmulatorAudioProcessor(); // Constructor
    ~PedalEmulatorAudioProcessor(); // Destructor

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override; // Not used
    bool producesMidi() const override; // Not used
    bool isMidiEffect() const override; // Not used
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioProcessorValueTreeState treeState;
    float Gain;
    APF apf0; //All pass filter
    Phaser phaser;
    PhaseShifter phaserShifter;
    static const int kChannels = 2; // 2 channels
protected:
    /*void updateParameters()
    {
        PhaseShifterParameters params = phaserShifter.getParameters();
        params.lfoRate_Hz = 1000.0;
        params.lfoDepth_Pct = 100.0;
        params.intensity_Pct = 75.0;
        phaserShifter.setParameters(params);
    }*/
    void updateParameters()
    {
        PhaserStruct phaserParams = phaser.getParameters();
        phaserParams.lfoRate = 1.0f;
        phaserParams.lfoDepth = 100.0f;
        phaserParams.intensity = 75.0f;

        phaser.setParameters(phaserParams);
    }
private:

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PedalEmulatorAudioProcessor)
};
