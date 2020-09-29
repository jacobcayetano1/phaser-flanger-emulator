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

//==============================================================================
PedalEmulatorAudioProcessorEditor::PedalEmulatorAudioProcessorEditor (PedalEmulatorAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    sliderAttach = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, GAIN_ID, gainSlider);
    
    gainSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    gainSlider.setRange(-60.0f, 0.0f, 0.01f); // min, max, increment
    //gainSlider.setValue(-20.0f); // Set starting value
    gainSlider.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    addAndMakeVisible(gainSlider);
    
    paramValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, "parameter", paramDial);
    
    paramDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    paramDial.setRange(0.0f, 1.0f);
    paramDial.setValue(0.5f);
    paramDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    //paramDial.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&paramDial);

    

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
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
    paramDial.setBounds(100, 90, 70, 70);
}

void PedalEmulatorAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &gainSlider) // If slider pointer is the gain slider
    {
        processor.Gain = gainSlider.getValue(); // Gain is set to slider's current value
        paramDial.setValue(processor.treeState.getParameter("parameter")->getValue(),juce::dontSendNotification);
    }
}
