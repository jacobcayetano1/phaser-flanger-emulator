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
#define PHASER_RATE_ID "phaser_rate"
#define PHASER_RATE_NAME "phaserRate"
#define FLANGER_DEPTH_ID "flanger_depth"
#define FLANGER_DEPTH_NAME "flangerDepth"
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
    treeState.createAndAddParameter(GAIN_ID, GAIN_NAME, GAIN_NAME, gainRange, 0.0f, nullptr, nullptr); // Gain parameter creation

    NormalisableRange<float> phaserRateRange(0.2f, 10.0f); // Range creation for rate
    treeState.createAndAddParameter(PHASER_RATE_ID, PHASER_RATE_NAME, PHASER_RATE_NAME, phaserRateRange, 1.0f, nullptr, nullptr); // Rate parameter creation
    
    NormalisableRange<float> flangerDepthRange(0.0f, 100.0f); // Range creation for depth
    treeState.createAndAddParameter(FLANGER_DEPTH_ID, FLANGER_DEPTH_NAME, FLANGER_DEPTH_NAME, flangerDepthRange, 100.0f, nullptr, nullptr); // Depth parameter creation
    
    NormalisableRange<float> drywetRange(0.0f, 100.0f); // Range creation for intensity
    treeState.createAndAddParameter(DRYWET_ID, DRYWET_NAME, DRYWET_NAME, drywetRange, 100.0f, nullptr, nullptr); // Intensity parameter creation
    
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
    //flanger.reset(sampleRate, 1);
    previousGain = Decibels::decibelsToGain(*treeState.getRawParameterValue(GAIN_ID)/20);
    /*
    float maxDelayTime = 0.02f + 0.02f;
    delayBufferSamples = (int)(maxDelayTime * (float)sampleRate) + 1;
    if (delayBufferSamples < 1)
    {
        delayBufferSamples = 1;
    }

    delayBufferChannels = getTotalNumInputChannels();
    delayBuffer.setSize(delayBufferChannels, delayBufferSamples);
    delayBuffer.clear();

    delayWritePosition = 0;
    lfoPhase = 0.0f;
    inverseSampleRate = 1.0f / (float)sampleRate;
    twoPi = 2.0f * M_PI;
    */
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

    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    //int lastChannel = 0;
    //int currentChannel = 0;

    int locWritePosition;
    float phaseVal;
    float phaseMain;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        //const float* inputData = buffer.getReadPointer(channel);
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
            /*
            const float in = channelData[sample];
            float out = 0.0f;

            float localDelayTime = (0.0025f + 0.001f * flanger.lfo(phase, 1)) * (float)getSampleRate();
            
            float readPosition = fmodf((float)localWritePosition - localDelayTime + (float)delayBufferSamples, delayBufferSamples);
            int localReadPosition = floorf(readPosition);

            // Cubic Interpolation
            float fraction = readPosition - (float)localReadPosition;
            float fractionSqrt = fraction * fraction;
            float fractionCube = fractionSqrt * fraction;

            float sample0 = delayData[(localReadPosition - 1 + delayBufferSamples) % delayBufferSamples];
            float sample1 = delayData[(localReadPosition + 0)];
            float sample2 = delayData[(localReadPosition + 1) % delayBufferSamples];
            float sample3 = delayData[(localReadPosition + 2) % delayBufferSamples];

            float a0 = -0.5f * sample0 + 1.5f * sample1 - 1.5f * sample2 + 0.5f * sample3;
            float a1 = sample0 - 2.5f * sample1 + 2.0f * sample2 - 0.5f * sample3;
            float a2 = -0.5f * sample0 + 0.5f * sample2;
            float a3 = sample1;
            out = a0 * fractionCube + a1 * fractionSqrt + a2 * fraction + a3;
            
            //channelData[sample] = in + out * (*treeState.getRawParameterValue(FLANGER_DEPTH_ID) /100.0f); //currentInverted;
            channelData[sample] = in + out * 1.0f * 1.0f;
            delayData[localWritePosition] = in + out * 0.1f; //* 0.5f;//currentFeedback;

            if (++localWritePosition >= delayBufferSamples)
                localWritePosition -= delayBufferSamples;

            phase += 10.0f* inverseSampleRate;
            if (phase >= 1.0f)
            {
                phase -= 1.0f;
            } */
            
            // Actual processing
            channelData[sample] = flanger.processAudioSample(channelData[sample], writePtr, phasePtr, getSampleRate());
            //channelData[sample] = phaser.processAudioSample(channelData[sample], channel, getSampleRate());
            //channelData[sample] = flanger.processAudioSample(channelData[sample], channel, getSampleRate());
        }
        if (channel == 0)
        {
            phaseMain = phaseVal;
        }
    }
    flanger.delayWritePosition = locWritePosition;
    flanger.lfoPhase = phaseMain;
 
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

//==============================================================================
/*float PedalEmulatorAudioProcessor::lfo(float phase, int waveform)
{
    float out = 0.0f;

    switch (waveform) {
    case waveformSine: {
        out = 0.5f + 0.5f * sinf(twoPi * phase);
        break;
    }
    case waveformTriangle: {
        if (phase < 0.25f)
            out = 0.5f + 2.0f * phase;
        else if (phase < 0.75f)
            out = 1.0f - 2.0f * (phase - 0.25f);
        else
            out = 2.0f * (phase - 0.75f);
        break;
    }
    case waveformSawtooth: {
        if (phase < 0.5f)
            out = 0.5f + phase;
        else
            out = phase - 0.5f;
        break;
    }
    case waveformInverseSawtooth: {
        if (phase < 0.5f)
            out = 0.5f - phase;
        else
            out = 1.5f - phase;
        break;
    }
    }

    return out;
}*/


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
