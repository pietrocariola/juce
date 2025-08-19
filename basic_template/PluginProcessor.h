/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
*/
class BasicTemplateAudioProcessor  : public juce::AudioProcessor
{
public:

    // constructor and destructor
    BasicTemplateAudioProcessor();
    ~BasicTemplateAudioProcessor() override;

    // other member functions
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    // member function where data processing occurs
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // creates AudioProcessorEditor (GUI)
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    // macro that expands into:
    // 1) non-copyable declaration avoiding unwanted copies of BasicTemplateAudioProcessorEditor
    // 2) memory leak detector
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicTemplateAudioProcessor)
};
