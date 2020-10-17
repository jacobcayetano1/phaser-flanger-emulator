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
#include "APF.h"

// Defines
#define GAIN_ID "gain"
#define GAIN_NAME "Gain"
#define DEPTH_ID "depth"
#define DEPTH_NAME "Depth"
#define RATE_ID "rate"
#define RATE_NAME "Rate"
#define DRYWET_ID "drywet"
#define DRYWET_NAME "DryWet"

//==============================================================================
PedalEmulatorAudioProcessor::PedalEmulatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
          #if ! JucePlugin_IsMidiEffect
          #if ! JucePlugin_IsSynth
                .withInput  ("Input",  AudioChannelSet::stereo(), true)
          #endif
                .withOutput ("Output", AudioChannelSet::stereo(), true)
          #endif
          ),
      treeState(*this,nullptr)
#endif
{
    // Create parameters here
    // treeState.createAndAddParameter(const String &parameterID, const String &parameterName, const String &parameterLabel={}, Category parameterCategory=AudioProcessorParameter::genericParameter)
    // Parameter name = parameter label
    NormalisableRange<float> gainRange(-60.0f, 0.0f); // Range creation for gain
    treeState.createAndAddParameter(GAIN_ID, GAIN_NAME, GAIN_NAME, gainRange, -20.0f, nullptr, nullptr); // Gain parameter creation

    NormalisableRange<float> depthRange(0.0f, 100.0f); // Range creation for depth
    treeState.createAndAddParameter(DEPTH_ID, DEPTH_NAME, DEPTH_NAME, depthRange, 100.0f, nullptr, nullptr); // Depth parameter creation
    
    NormalisableRange<float> rateRange(0.2f, 10.0f); // Range creation for rate
    treeState.createAndAddParameter(RATE_ID, RATE_NAME, RATE_NAME, rateRange, 0.5f, nullptr, nullptr); // Rate parameter creation

    NormalisableRange<float> drywetRange(0.0f, 100.0f); // Range creation for intensity
    treeState.createAndAddParameter(DRYWET_ID, DRYWET_NAME, DRYWET_NAME, drywetRange, 50.0f, nullptr, nullptr); // Intensity parameter creation
    
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
    /*
    apf0.reset(sampleRate,0); // Reset channel 0
    apf0.reset(sampleRate,1); // Reset channel 1
    for (int i = 0; i < kChannels; i++)
    {
        apf0.biquad.stateArray[i][0] == 0.0f;
        apf0.biquad.stateArray[i][1] == 0.0f;
        apf0.biquad.stateArray[i][2] == 0.0f;
        apf0.biquad.stateArray[i][3] == 0.0f;
    }
    */
    phaser.reset(sampleRate, 0);
    phaser.reset(sampleRate, 1);
    flanger.reset(sampleRate, 0);
    flanger.reset(sampleRate, 1);
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    int lastChannel = 0;
    int currentChannel = 0;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) // Goes through all samples in buffer
        {
            // *Note: ASPIK works with double while JUCE works with float, how to integrate?
            
            // Test processing
            //channelData[sample] = channelData[sample] * Decibels::decibelsToGain(-20.0);
            
            // Actual processing
            //apf0.setParameters(apf0.getParameters()); // includes calculateFilterCoeffs()
            //treeState.getParameter("parameter")->setValue(apf0.coeffArray[a0]); // Used to check coeff value
            //channelData[sample] = apf0.processAudioSample(channelData[sample],channel) * Decibels::decibelsToGain(Gain);
            //phaser.setParameters(phaser.getParameters());
            updateParameters();
            channelData[sample] = phaser.processAudioSample(channelData[sample], channel);
            channelData[sample] = flanger.processAudioSample(channelData[sample], channel);
            channelData[sample] = channelData[sample] * Decibels::decibelsToGain(Gain);
        }
    }
}

//==============================================================================
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
