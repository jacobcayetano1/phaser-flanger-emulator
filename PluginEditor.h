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
class PedalEmulatorAudioProcessorEditor : public AudioProcessorEditor,
    public Slider::Listener, public Button::Listener
{
public:
    PedalEmulatorAudioProcessorEditor (PedalEmulatorAudioProcessor&);
    ~PedalEmulatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    
    void resized() override;

    void sliderValueChanged(Slider* slider) override;

    void buttonClicked(Button* button) override;
    
    // Scoped Pointers
    // Overall
    ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> volumeSliderAttach;
    ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> dryWetSliderAttach;
    ScopedPointer <AudioProcessorValueTreeState::ButtonAttachment> routePhaserFirstValue;
    
    // Phaser
    ScopedPointer <AudioProcessorValueTreeState::ButtonAttachment> bypassPhaserValue;
    ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> phaserRateValue;
    ScopedPointer <AudioProcessorValueTreeState::ButtonAttachment> phaserFeedbackSwitchValue;
    
    // Flanger
    ScopedPointer <AudioProcessorValueTreeState::ButtonAttachment> bypassFlangerValue;
    ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> flangerDepthValue;
    ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> flangerRateValue;
    ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> flangerFeedbackValue;
    ScopedPointer <AudioProcessorValueTreeState::ButtonAttachment> flangerInvertedValue;

private:
    Slider gainSlider;
    Label volumeSliderLabel;
    Slider dryWetSlider;
    Label dryWetSliderLabel;
    ToggleButton routePhaserFirstButton;
    Label phaserLabel;
    Label flangerLabel;

    // Phaser
    Slider phaserRateDial;
    Label phaserRateLabel;
    ToggleButton bypassPhaserButton;
    ToggleButton phaserFeedbackSwitchButton;

    // Flanger
    ToggleButton bypassFlangerButton;
    Slider flangerDepthDial;
    Label flangerDepthLabel;
    Slider flangerRateDial;
    Label flangerRateLabel;
    Slider flangerFeedbackDial;
    Label flangerFeedbackLabel;
    ToggleButton flangerInvertedButton;

    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PedalEmulatorAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PedalEmulatorAudioProcessorEditor)
};
