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

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define GAIN_ID "gain"
#define FLANGER_DEPTH_ID "flanger_depth"
#define PHASER_RATE_ID "phaser_rate"
#define DRYWET_ID "drywet"

//==============================================================================
PedalEmulatorAudioProcessorEditor::PedalEmulatorAudioProcessorEditor (PedalEmulatorAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Master Gain
    volumeSliderAttach = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, GAIN_ID, gainSlider);
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 100, 20);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setRange(-60.0f, 0.0f, 0.01f); // min, max, increment
    gainSlider.setSkewFactorFromMidPoint(-20.0f);
    addAndMakeVisible(volumeSliderLabel);
    volumeSliderLabel.setText("Output Volume", juce::dontSendNotification);
    volumeSliderLabel.attachToComponent(&gainSlider, false);
    gainSlider.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    
    // Phaser Rate
    phaserRateValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, PHASER_RATE_ID, phaserRateDial);
    phaserRateDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    phaserRateDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    phaserRateDial.setRange(0.2f, 10.0f, 0.01f);
    phaserRateDial.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    addAndMakeVisible(phaserRateDial);
    
    // Flanger Depth
    flangerDepthValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, FLANGER_DEPTH_ID, flangerDepthDial);
    flangerDepthDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    flangerDepthDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    flangerDepthDial.setRange(0.0f, 100.0f, 0.01f);
    flangerDepthDial.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    addAndMakeVisible(flangerDepthDial);

    // Dry/Wet mix
    drywetValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, DRYWET_ID, drywetDial);
    drywetDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    drywetDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    drywetDial.setRange(0.0f, 100.0f);
    drywetDial.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    addAndMakeVisible(drywetDial);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 500);
}

PedalEmulatorAudioProcessorEditor::~PedalEmulatorAudioProcessorEditor()
{
}

//==============================================================================
void PedalEmulatorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black); // color
    /*
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
    */
}

void PedalEmulatorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    gainSlider.setBounds((getWidth() / 2 - 50), (getHeight() / 2 - 75), 100, 150);
    flangerDepthDial.setBounds(500, 90, 100, 100);
    phaserRateDial.setBounds(10, 10, 100, 100);
    drywetDial.setBounds(600, 90, 100, 100);
}

void PedalEmulatorAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &gainSlider || slider == &flangerDepthDial || slider == &phaserRateDial || slider == &drywetDial) // If slider pointer is the gain slider
    {
        processor.Gain = gainSlider.getValue(); // Gain is set to slider's current value
        processor.flangerDepth = flangerDepthDial.getValue(); // Depth set to dial's current value
        processor.phaserRate = phaserRateDial.getValue(); // Rate set to dial's current value
        processor.DryWet = drywetDial.getValue(); // Intensity set to dial's current value
    }
}
