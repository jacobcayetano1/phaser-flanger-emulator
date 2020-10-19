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
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PedalEmulatorAudioProcessorEditor  : public AudioProcessorEditor,
    public Slider::Listener
{
public:
    PedalEmulatorAudioProcessorEditor (PedalEmulatorAudioProcessor&);
    ~PedalEmulatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void sliderValueChanged(Slider* slider) override;
    
    ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> sliderAttach;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> depthValue;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> rateValue;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> drywetValue;

private:
    Slider gainSlider;
    Slider depthDial;
    Slider rateDial;
    Slider drywetDial;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PedalEmulatorAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PedalEmulatorAudioProcessorEditor)
};
