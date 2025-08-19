/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BasicTemplateAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    // constructor and destructor
    BasicTemplateAudioProcessorEditor (BasicTemplateAudioProcessor&);
    ~BasicTemplateAudioProcessorEditor() override;

    // other virtual member functions to be overriden
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // this reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BasicTemplateAudioProcessor& audioProcessor;

    // macro that expands into:
    // 1) non-copyable declaration avoiding unwanted copies of BasicTemplateAudioProcessorEditor
    // 2) memory leak detector
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicTemplateAudioProcessorEditor)
};
