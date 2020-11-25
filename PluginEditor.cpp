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
#define ROUTE_PHASER_FIRST_ID "route_phaser_first"
#define DRYWET_ID "dry_wet"

#define PHASER_RATE_ID "phaser_rate"
#define BYPASS_PHASER_ID "bypass_phaser"
#define PHASER_FEEDBACK_SWITCH_ID "phaser_feedback_switch"

#define BYPASS_FLANGER_ID "bypass_flanger"
#define FLANGER_DEPTH_ID "flanger_depth"
#define FLANGER_RATE_ID "flanger_rate"
#define FLANGER_FEEDBACK_ID "flanger_feedback"
#define FLANGER_INVERTED_ID "flanger_inverted"

//==============================================================================
PedalEmulatorAudioProcessorEditor::PedalEmulatorAudioProcessorEditor (PedalEmulatorAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Effect labels
    addAndMakeVisible(phaserLabel);
    phaserLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    phaserLabel.setText("PHASER", juce::dontSendNotification);
    addAndMakeVisible(flangerLabel);
    flangerLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    flangerLabel.setText("FLANGER", juce::dontSendNotification);
    
    // Master Gain
    volumeSliderAttach = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, GAIN_ID, gainSlider);
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 100, 20);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setRange(-60.0f, 0.0f, 0.01f); // min, max, increment
    gainSlider.setSkewFactorFromMidPoint(-20.0f);
    addAndMakeVisible(volumeSliderLabel);
    volumeSliderLabel.setText("Gain", juce::dontSendNotification);
    volumeSliderLabel.attachToComponent(&gainSlider, false);
    volumeSliderLabel.setJustificationType(Justification::centredTop);
    gainSlider.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    
    // Dry/Wet Mix
    dryWetSliderAttach = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, DRYWET_ID, dryWetSlider);
    addAndMakeVisible(dryWetSlider);
    dryWetSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    dryWetSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 100, 20);
    dryWetSlider.setTextValueSuffix(" %");
    dryWetSlider.setRange(0.0f, 100.0f, 0.01f); // min, max, increment
    addAndMakeVisible(dryWetSliderLabel);
    dryWetSliderLabel.setText("Dry/Wet Mix", juce::dontSendNotification);
    dryWetSliderLabel.attachToComponent(&dryWetSlider, false);
    dryWetSliderLabel.setJustificationType(Justification::centredTop);
    dryWetSlider.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes

    // Bypass Phaser
    bypassPhaserValue = new AudioProcessorValueTreeState::ButtonAttachment(processor.treeState, BYPASS_PHASER_ID, bypassPhaserButton);
    addAndMakeVisible(bypassPhaserButton);
    bypassPhaserButton.setButtonText("Bypass Phaser");
    //bypassPhaserButton.setToggleState(false, juce::dontSendNotification);
    bypassPhaserButton.addListener(this);

    // Bypass Flanger
    bypassFlangerValue = new AudioProcessorValueTreeState::ButtonAttachment(processor.treeState, BYPASS_FLANGER_ID, bypassFlangerButton);
    addAndMakeVisible(bypassFlangerButton);
    bypassFlangerButton.setButtonText("Bypass Flanger");
    //bypassFlangerButton.setToggleState(false, juce::dontSendNotification);
    bypassFlangerButton.addListener(this);

    // Route Phaser First
    routePhaserFirstValue = new AudioProcessorValueTreeState::ButtonAttachment(processor.treeState, ROUTE_PHASER_FIRST_ID, routePhaserFirstButton);
    addAndMakeVisible(routePhaserFirstButton);
    routePhaserFirstButton.setButtonText("Phaser->Flanger");
    routePhaserFirstButton.setToggleState(true, juce::dontSendNotification);
    routePhaserFirstButton.addListener(this);

    // Phaser Rate
    phaserRateValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, PHASER_RATE_ID, phaserRateDial);
    addAndMakeVisible(phaserRateDial);
    phaserRateDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    phaserRateDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    phaserRateDial.setTextValueSuffix(" Hz");
    phaserRateDial.setRange(0.2f, 10.0f, 0.01f);
    addAndMakeVisible(phaserRateLabel);
    phaserRateLabel.setText("Rate", juce::dontSendNotification);
    phaserRateLabel.attachToComponent(&phaserRateDial, false);
    phaserRateLabel.setJustificationType(Justification::centredTop);
    phaserRateDial.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes

    // Phaser Feedback Switch
    phaserFeedbackSwitchValue = new AudioProcessorValueTreeState::ButtonAttachment(processor.treeState, PHASER_FEEDBACK_SWITCH_ID, phaserFeedbackSwitchButton);
    addAndMakeVisible(phaserFeedbackSwitchButton);
    phaserFeedbackSwitchButton.setButtonText("Feedback Switch");
    //phaserFeedbackSwitchButton.setToggleState(true, juce::dontSendNotification);
    phaserFeedbackSwitchButton.addListener(this);
    
    // Flanger Depth
    flangerDepthValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, FLANGER_DEPTH_ID, flangerDepthDial);
    addAndMakeVisible(flangerDepthDial);
    flangerDepthDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    flangerDepthDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    flangerDepthDial.setTextValueSuffix(" %");
    flangerDepthDial.setRange(0.0f, 1.0f);
    addAndMakeVisible(flangerDepthLabel);
    flangerDepthLabel.setText("Depth", juce::dontSendNotification);
    flangerDepthLabel.attachToComponent(&flangerDepthDial, false);
    flangerDepthLabel.setJustificationType(Justification::centredTop);
    flangerDepthDial.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    
    // Flanger Rate
    flangerRateValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, FLANGER_RATE_ID, flangerRateDial);
    addAndMakeVisible(flangerRateDial);
    flangerRateDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    flangerRateDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    flangerRateDial.setTextValueSuffix(" %");
    flangerRateDial.setRange(0.0f, 10.0f);
    addAndMakeVisible(flangerRateLabel);
    flangerRateLabel.setText("Rate", juce::dontSendNotification);
    flangerRateLabel.attachToComponent(&flangerRateDial, false);
    flangerRateLabel.setJustificationType(Justification::centredTop);
    flangerRateDial.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes
    
    // Flanger Feedback
    flangerFeedbackValue = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, FLANGER_FEEDBACK_ID, flangerFeedbackDial);
    addAndMakeVisible(flangerFeedbackDial);
    flangerFeedbackDial.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    flangerFeedbackDial.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    flangerFeedbackDial.setTextValueSuffix(" %");
    flangerFeedbackDial.setRange(0.0f, 0.8f);
    addAndMakeVisible(flangerFeedbackLabel);
    flangerFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    flangerFeedbackLabel.attachToComponent(&flangerFeedbackDial, false);
    flangerFeedbackLabel.setJustificationType(Justification::centredTop);
    flangerFeedbackDial.addListener(this); //this points to AudioProcessorEditor class because that is the class that is listening for changes

    // Flanger Inverted
    flangerInvertedValue = new AudioProcessorValueTreeState::ButtonAttachment(processor.treeState, FLANGER_INVERTED_ID, flangerInvertedButton);
    addAndMakeVisible(flangerInvertedButton);
    flangerInvertedButton.setButtonText("Inverted");
    //flangerInvertedButton.setToggleState(true, juce::dontSendNotification);
    flangerInvertedButton.addListener(this);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 460);
}

PedalEmulatorAudioProcessorEditor::~PedalEmulatorAudioProcessorEditor()
{
}

//==============================================================================
void PedalEmulatorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black); // color
    g.setColour(juce::Colours::dodgerblue);
    juce::Line<float> line0(juce::Point<float>(0, 225), juce::Point<float>(430, 225));
    juce::Line<float> line1(juce::Point<float>(430, 0), juce::Point<float>(430, 460));
    
    g.drawLine(line0, 2.0f);
    g.drawLine(line1, 2.0f);

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
    // Large labels
    phaserLabel.setBounds(150, 5, 200, 30);
    flangerLabel.setBounds(145, 230, 200, 30);
    
    // Overall controls
    gainSlider.setBounds(460, 90, 100, 160);
    dryWetSlider.setBounds(460, 280, 100, 160);
    routePhaserFirstButton.setBounds(440, 20, 140, 30);

    // Phaser
    phaserRateDial.setBounds(57, 110, 120, 100);
    bypassPhaserButton.setBounds(10, 40, 150, 30);
    phaserFeedbackSwitchButton.setBounds(180, 130, 150, 30);

    // Flanger
    bypassFlangerButton.setBounds(10, 270, 150, 30);
    flangerDepthDial.setBounds(10, 340, 120, 100);
    flangerRateDial.setBounds(110, 340, 120, 100);
    flangerFeedbackDial.setBounds(210, 340, 120, 100);
    flangerInvertedButton.setBounds(310, 360, 100, 30);
}

void PedalEmulatorAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &gainSlider || slider == &flangerDepthDial || slider == &phaserRateDial ||
        slider == &flangerRateDial || slider == &flangerFeedbackDial || slider == &dryWetSlider)
    {
        // Gain
        processor.Gain = gainSlider.getValue(); // Gain is set to slider's current value

        // Dry/Wet Mix
        processor.dryWetMix = dryWetSlider.getValue();
        
        // Phaser
        processor.phaserRate = phaserRateDial.getValue(); // Rate set to dial's current value

        // Flanger
        processor.flangerDepth = flangerDepthDial.getValue(); // Depth set to dial's current value
        processor.flangerRate = flangerRateDial.getValue(); // Intensity set to dial's current value
        processor.flangerFeedback = flangerFeedbackDial.getValue(); // Feedback set to dial's current value
    }
}


void PedalEmulatorAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &flangerInvertedButton || button == &bypassFlangerButton || button == &bypassPhaserButton || 
        button == &routePhaserFirstButton || button == &phaserFeedbackSwitchButton)
    {
        // Bypass Phaser
        processor.bypassPhaser = bypassPhaserButton.getToggleState();

        // Bypass Flanger
        processor.bypassFlanger = bypassFlangerButton.getToggleState();

        // Route Phaser First
        processor.routePhaserFirst = routePhaserFirstButton.getToggleState();
        if (routePhaserFirstButton.getToggleState() == true)
        {
            routePhaserFirstButton.setButtonText("Phaser->Flanger");
        }
        else
        {
            routePhaserFirstButton.setButtonText("Phaser<-Flanger");
        }
        
        // Phaser
        processor.phaserSwitch = phaserFeedbackSwitchButton.getToggleState();

        // Flanger
        processor.flangerInverted = flangerInvertedButton.getToggleState(); // Inverted state set to button's current state
    }
}
