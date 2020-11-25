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
//#define _USE_MATH_DEFINES
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Flanger.h"
//#include <cmath>

// Defines
#define GAIN_ID "gain"
#define GAIN_NAME "Gain"
#define ROUTE_PHASER_FIRST_ID "route_phaser_first"
#define ROUTE_PHASER_FIRST_NAME "routePhaserFirst"
#define DRYWET_ID "dry_wet"
#define DRYWET_NAME "dryWet"

#define PHASER_RATE_ID "phaser_rate"
#define PHASER_RATE_NAME "phaserRate"
#define BYPASS_PHASER_ID "bypass_phaser"
#define BYPASS_PHASER_NAME "bypassPhaser"
#define PHASER_FEEDBACK_SWITCH_ID "phaser_feedback_switch"
#define PHASER_FEEDBACK_SWITCH_NAME "phaserFeedbackSwitch"

#define BYPASS_FLANGER_ID "bypass_flanger"
#define BYPASS_FLANGER_NAME "bypassFlanger"
#define FLANGER_DEPTH_ID "flanger_depth"
#define FLANGER_DEPTH_NAME "flangerDepth"
#define FLANGER_RATE_ID "flanger_rate"
#define FLANGER_RATE_NAME "flangerRate"
#define FLANGER_FEEDBACK_ID "flanger_feedback"
#define FLANGER_FEEDBACK_NAME "flangerFeedback"
#define FLANGER_INVERTED_ID "flanger_inverted"
#define FLANGER_INVERTED_NAME "flangerInverted"

using Parameter = AudioProcessorValueTreeState::Parameter;

//==============================================================================
PedalEmulatorAudioProcessor::PedalEmulatorAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (BusesProperties()
          #if ! JucePlugin_IsMidiEffect
          #if ! JucePlugin_IsSynth
                .withInput  ("Input",  AudioChannelSet::stereo(), true)
          #endif
                .withOutput ("Output", AudioChannelSet::stereo(), true)
          #endif
          ),
           treeState(*this, nullptr)
#endif
{
    // Create parameters here
    // treeState.createAndAddParameter(const String &parameterID, const String &parameterName, const String &parameterLabel={}, Category parameterCategory=AudioProcessorParameter::genericParameter)
    // Parameter name = parameter label

    NormalisableRange<float> gainRange(-60.0f, 0.0f); // Range creation for gain
    treeState.createAndAddParameter(GAIN_ID, GAIN_NAME, GAIN_NAME, gainRange, 0.0f, nullptr, nullptr); // Gain parameter creation

    NormalisableRange<float> dryWetRange(0.0f, 100.0f);
    treeState.createAndAddParameter(DRYWET_ID, DRYWET_NAME, DRYWET_NAME, dryWetRange, 100.0f, nullptr, nullptr);

    NormalisableRange<float> phaserRateRange(0.2f, 10.0f); // Range creation for rate
    treeState.createAndAddParameter(PHASER_RATE_ID, PHASER_RATE_NAME, PHASER_RATE_NAME, phaserRateRange, 1.0f, nullptr, nullptr); // Rate parameter creation
    
    NormalisableRange<float> flangerDepthRange(0.0f, 1.0f); // Range creation for depth
    treeState.createAndAddParameter(FLANGER_DEPTH_ID, FLANGER_DEPTH_NAME, FLANGER_DEPTH_NAME, flangerDepthRange, 1.0f, nullptr, nullptr); // Depth parameter creation
    
    NormalisableRange<float> flangerRateRange(0.0f, 10.0f); // Range creation for intensity
    treeState.createAndAddParameter(FLANGER_RATE_ID, FLANGER_RATE_NAME, FLANGER_RATE_NAME, flangerRateRange, 1.0f, nullptr, nullptr); // Intensity parameter creation

    NormalisableRange<float> flangerFeedbackRange(0.0f, 0.8f); // Range creation for intensity
    treeState.createAndAddParameter(FLANGER_FEEDBACK_ID, FLANGER_FEEDBACK_NAME, FLANGER_FEEDBACK_NAME, flangerFeedbackRange, 0.5f, nullptr, nullptr); // Intensity parameter creation

    Range<float> flangerInvertedRange(-1.0f, 1.0f);
    treeState.createAndAddParameter(FLANGER_INVERTED_ID, FLANGER_INVERTED_NAME, FLANGER_INVERTED_NAME, flangerInvertedRange, 1.0f, nullptr, nullptr);

    // Buttons
    Range<float> buttonRange(0.0f, 1.0f);
    treeState.createAndAddParameter(BYPASS_PHASER_ID, BYPASS_PHASER_NAME, BYPASS_PHASER_NAME, buttonRange, 0.0f, nullptr, nullptr);
    treeState.createAndAddParameter(BYPASS_FLANGER_ID, BYPASS_FLANGER_NAME, BYPASS_FLANGER_NAME, buttonRange, 0.0f, nullptr, nullptr);
    treeState.createAndAddParameter(ROUTE_PHASER_FIRST_ID, ROUTE_PHASER_FIRST_NAME, ROUTE_PHASER_FIRST_NAME, buttonRange, 0.0f, nullptr, nullptr);
    treeState.createAndAddParameter(PHASER_FEEDBACK_SWITCH_ID, PHASER_FEEDBACK_SWITCH_NAME, PHASER_FEEDBACK_SWITCH_NAME, buttonRange, 0.0f, nullptr, nullptr);
    
    treeState.state = ValueTree("savedParams"); // Used for saving parameters
}

PedalEmulatorAudioProcessor::~PedalEmulatorAudioProcessor()
{
}

//==============================================================================
const String PedalEmulatorAudioProcessor::getName() const
{
    return JucePlugin_Name; // PedalEmulator
}

bool PedalEmulatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PedalEmulatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PedalEmulatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PedalEmulatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PedalEmulatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PedalEmulatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PedalEmulatorAudioProcessor::setCurrentProgram (int index)
{
}

const String PedalEmulatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void PedalEmulatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void PedalEmulatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    phaser.reset(sampleRate, 0);
    phaser.reset(sampleRate, 1);
    flanger.reset(sampleRate, getTotalNumInputChannels());
    previousGain = Decibels::decibelsToGain(*treeState.getRawParameterValue(GAIN_ID)/20);
}

void PedalEmulatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PedalEmulatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PedalEmulatorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    float currentGain = pow(10, *treeState.getRawParameterValue(GAIN_ID) / 20);

    // Gain processing done across buffer outside of loop
    if (currentGain == previousGain)
    {
        buffer.applyGain(currentGain);
    }
    else {
        buffer.applyGainRamp(0, numSamples, previousGain, currentGain);
        previousGain = currentGain;
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        flanger.delayData = flanger.delayBuffer.getWritePointer(channel);
        locWritePosition = flanger.delayWritePosition;
        phaseVal = flanger.lfoPhase;
        int* writePtr = &locWritePosition;
        float* phasePtr = &phaseVal;
        
        /*
        if (false && channel != 0)
        {
            phase = fmodf(phase + 0.25f, 1.0f);
        }*/ //stereo option
        
        for (int sample = 0; sample < numSamples; ++sample) // Goes through all samples in buffer
        {
            // *Note: ASPIK works with double while JUCE works with float, how to integrate?
            updateParameters(channel);
            
            // Actual processing
            if (routePhaserFirst)
            {
                if (!bypassPhaser)
                {
                    channelData[sample] = ((dryWetMix * 0.01f) * phaser.processAudioSample(channelData[sample], channel, getSampleRate())) +
                                   ((1.0f - (dryWetMix * 0.01f)) * channelData[sample]);
                    //channelData[sample] = phaser.processAudioSample(channelData[sample], channel, getSampleRate());
                }
                if (!bypassFlanger)
                {
                    channelData[sample] = ((dryWetMix * 0.01f) * flanger.processAudioSample(channelData[sample], writePtr, phasePtr, getSampleRate(), channel)) +
                        ((1.0f - (dryWetMix * 0.01f)) * channelData[sample]);
                    //channelData[sample] = flanger.processAudioSample(channelData[sample], writePtr, phasePtr, getSampleRate(), channel);
                }
            }
            else
            {
                if (!bypassFlanger)
                {
                    channelData[sample] = ((dryWetMix * 0.01f) * flanger.processAudioSample(channelData[sample], writePtr, phasePtr, getSampleRate(), channel)) +
                        ((1.0f - (dryWetMix * 0.01f)) * channelData[sample]);
                    //channelData[sample] = flanger.processAudioSample(channelData[sample], writePtr, phasePtr, getSampleRate(), channel);
                }
                if (!bypassPhaser)
                {
                    channelData[sample] = ((dryWetMix * 0.01f) * phaser.processAudioSample(channelData[sample], channel, getSampleRate())) +
                        ((1.0f - (dryWetMix * 0.01f)) * channelData[sample]);
                    //channelData[sample] = phaser.processAudioSample(channelData[sample], channel, getSampleRate());
                }
                
            }
        }
        if (channel == 0)
        {
            phaseMain = phaseVal; // Flanger variables for LFO
        }
    }
    flanger.delayWritePosition = locWritePosition;
    flanger.lfoPhase = phaseMain;
 
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

bool PedalEmulatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PedalEmulatorAudioProcessor::createEditor()
{
    return new PedalEmulatorAudioProcessorEditor (*this);
}

//==============================================================================
void PedalEmulatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // This is done with XML to save plugin state on a project
    auto state = treeState.copyState();
    std::unique_ptr <XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PedalEmulatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr <XmlElement> theParams(getXmlFromBinary(data, sizeInBytes));
    if (theParams != nullptr)
    {
        if (theParams->hasTagName(treeState.state.getType()))
        {treeState.state = ValueTree::fromXml(*theParams);}
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PedalEmulatorAudioProcessor();
}
