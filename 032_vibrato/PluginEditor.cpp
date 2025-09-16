#include "PluginProcessor.h"
#include "PluginEditor.h"

MyAudioProcessorEditor::MyAudioProcessorEditor (MyAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Para evitar warnings
    juce::ignoreUnused(audioProcessor);
    
    // Define o tamanho do editor
    setSize (400, 300);
}

MyAudioProcessorEditor::~MyAudioProcessorEditor() {}

void MyAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Preenche a tela com uma cor solida
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

    // TODO: Define nome do plugin na tela. Aqui usamos "Plugin" apenas
    g.drawFittedText ("Plugin", getLocalBounds(), juce::Justification::centred, 1);
}

// Funcao em que sao definidas posicoes customizadas dos elementos
void MyAudioProcessorEditor::resized() {}