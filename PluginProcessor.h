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
#include "Flanger.h"
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

    // Parameters
    AudioProcessorValueTreeState treeState;
    float Gain;
    float flangerDepth;
    float phaserRate;
    float DryWet;
    
    float previousGain;

    Phaser phaser;
    Flanger flanger;
    static const int kChannels = 2; // 2 channels

    //float s0, s1, s2, s3;
    //float* delayData;

    /*
    AudioSampleBuffer delayBuffer;
    int delayBufferSamples;
    int delayBufferChannels;
    int delayWritePosition;

    float lfoPhase;
    float inverseSampleRate;
    float twoPi;
    */

protected:
    
    void updateParameters(int channel)
    {
        PhaserStruct phaserParams = phaser.getParameters();
        //ModulatedDelayParameters flangerParams = flanger.getParameters();
        // Change to user controlled parameters
        // --- Phaser
        phaserParams.lfoRate = phaserRate;
        //phaserParams.drywet = DryWet; // Do not allow user to change intensity, messes up sound
        // --- Flanger
        //flangerParams.lfoDepth_Pct = flangerDepth;
        //flangerParams.lfoRate_Hz = 10.0f;
        // Higher depth and rate cause noise and artifacts

        phaser.setParameters(phaserParams);
        //flanger.setParameters(flangerParams,channel);
    }
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PedalEmulatorAudioProcessor)
};
