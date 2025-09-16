//==============================================================================
// PluginEditor.h: definicao do PluginEditor
//==============================================================================

#pragma once

#include "PluginProcessor.h"

class MyAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MyAudioProcessorEditor (MyAudioProcessor&);
    ~MyAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Referencia para o editor acessar o objeto MyAudioProcessor que o criou
    MyAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyAudioProcessorEditor)
};