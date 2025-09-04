/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// constructor
FakeSidechainAudioProcessorEditor::FakeSidechainAudioProcessorEditor (FakeSidechainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 200);
}

// destructor
FakeSidechainAudioProcessorEditor::~FakeSidechainAudioProcessorEditor()
{
}

//==============================================================================

void FakeSidechainAudioProcessorEditor::paint (juce::Graphics& g)
{
    // fill the background with a solid colour
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    // set colour to white
    g.setColour (juce::Colours::white);
    
    // set font size
    g.setFont(15.0f);
    
    // Draw Hello world text
    g.drawFittedText("Hello World", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void FakeSidechainAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
