/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <iostream>
#include <atomic>
#include <cstdint>

#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
*/
class FakeSidechainAudioProcessor  : public juce::AudioProcessor
{
public:

    // constructor and destructor
    FakeSidechainAudioProcessor();
    ~FakeSidechainAudioProcessor() override;

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

    // pietro ======================================================================
    int envelopeIndex_;    
    double envelopeWindowSize_;
    std::atomic<int64_t> currentBar_ {0};
    std::atomic<int64_t> barCounter_ {0};

    float FakeSidechainAudioProcessor::volumeEnvelope(int envelopeIndex, int envelopeWindowSize);
    int64_t FakeSidechainAudioProcessor::getBar();

private:
    // macro that expands into:
    // 1) non-copyable declaration avoiding unwanted copies of FakeSidechainAudioProcessorEditor
    // 2) memory leak detector
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FakeSidechainAudioProcessor)
};
