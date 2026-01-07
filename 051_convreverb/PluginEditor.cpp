#include "PluginProcessor.h"
#include "PluginEditor.h"

MyAudioProcessorEditor::MyAudioProcessorEditor (MyAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), apvts(vts)
{
    // Para evitar warnings
    juce::ignoreUnused(audioProcessor);
    
    loadButton.setButtonText("Load IR");
    loadButton.onClick = [this] { loadIR(); };
    
    addAndMakeVisible(loadButton);

    wetDryLabel.setText("Dry/Wet Mix", juce::dontSendNotification);
    addAndMakeVisible(wetDryLabel);
 
    addAndMakeVisible(wetDrySlider);
    wetDryAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "wet_dry", wetDrySlider));

    // Define o tamanho do editor
    setSize (400, 150);
}

MyAudioProcessorEditor::~MyAudioProcessorEditor() {}

void MyAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Preenche a tela com uma cor solida
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

    // TODO: Define nome do plugin na tela. Aqui usamos "Plugin" apenas
    g.drawFittedText ("Plugin", getLocalBounds(), juce::Justification::centredTop, 1);
}

// Funcao em que sao definidas posicoes customizadas dos elementos
void MyAudioProcessorEditor::resized() 
{
    wetDryLabel.setBounds(10, getHeight() / 10 + 10, 100, 20);
    wetDrySlider.setBounds(110, getHeight() / 10 + 10, 280, 20);
    loadButton.setBounds(10, getHeight() / 10 + 40, 100, 20);
}

void MyAudioProcessorEditor::loadIR()
{
    chooser = std::make_unique<juce::FileChooser>(
        "Select IR file",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif",
        false);

    auto chooserFlags = juce::FileBrowserComponent::openMode | 
        juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        const juce::File file = fc.getResult();

        if (file != juce::File{})
        {
            DBG("file selected" << file.getFileName());
            audioProcessor.loadImpulseResponse(file);
        }
    });
}
