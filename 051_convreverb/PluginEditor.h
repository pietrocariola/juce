//==============================================================================
// PluginEditor.h: definicao do PluginEditor
//==============================================================================

#pragma once

#include "PluginProcessor.h"

class MyAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MyAudioProcessorEditor (MyAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~MyAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Referencia para o editor acessar o objeto MyAudioProcessor que o criou
    MyAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts;

    juce::Label wetDryLabel;
    juce::Slider wetDrySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetDryAttachment;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::TextButton loadButton;
    void loadIR();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyAudioProcessorEditor)
};
