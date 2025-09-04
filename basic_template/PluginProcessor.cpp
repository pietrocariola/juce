#include "PluginProcessor.h"

BasicTemplateAudioProcessor::BasicTemplateAudioProcessor()
/* If JucePlugin_PreferredChannelConfigurations is not defined, 
the constructor uses JUCE’s BusesProperties to tell the base 
AudioProcessor class what audio channels (buses) the plugin will use.*/
     : AudioProcessor (
        #ifndef JucePlugin_PreferredChannelConfigurations // set in cmake
            BusesProperties()
                // MidiEffect does not need audio input/output
                #if ! JucePlugin_IsMidiEffect // set in cmake
                    // Synth does not need audio input, only output
                    #if ! JucePlugin_IsSynth // set in cmake
                        // adds audio input
                        // true means enabled by default
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                    #endif
                    // adds audio output
                    // true means enabled by default
                    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                #endif         
        #endif
        ), apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    /* 
        - member initialize list passes parameters before initialization
        - constructor body runs after initilization
    */
    gain_ = 1.0f;
    castParameter(apvts, ParamID::gain, gainParam);
    apvts.state.addListener(this);
}


BasicTemplateAudioProcessor::~BasicTemplateAudioProcessor()
{
    apvts.state.removeListener(this);
}

const juce::String BasicTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput // set in cmake
    return true;
   #else
    return false;
   #endif
}

bool BasicTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput // set in cmake
    return true;
   #else
    return false;
   #endif
}

bool BasicTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect // pietro: set in cmake
    return true;
   #else
    return false;
   #endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicTemplateAudioProcessor::isBusesLayoutSupported (
    const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect // set in cmake
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Support only mono or stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth // set in cmake
    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}
#endif

double BasicTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicTemplateAudioProcessor::getNumPrograms()
{
    return 1;   
}

int BasicTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicTemplateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicTemplateAudioProcessor::changeProgramName (int index, 
    const juce::String& newName)
{
}

void BasicTemplateAudioProcessor::prepareToPlay (double sampleRate, 
    int samplesPerBlock)
{
}

void BasicTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, 
    juce::MidiBuffer& midiMessages)
{
    // MIDI
    juce::ignoreUnused(midiMessages); // audio project will not use midi

    // AUDIO
    // loop through channels
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        // pointer to the channel 
        auto* channelData = buffer.getWritePointer(channel);
        
        // loop through the samples in the buffer
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= gain_;
        }
    }

    bool expected = true;
    if (parametersChanged.compare_exchange_strong(expected, false)) 
    {
        updateParameters();
    } 
}

void BasicTemplateAudioProcessor::updateParameters() 
{
    gain_ = gainParam->get();
}

void BasicTemplateAudioProcessor::releaseResources()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout 
    BasicTemplateAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamID::gain,
        "Gain", juce::NormalisableRange(0.0f, 2.0f), 1.0f));

    return layout;
}

void BasicTemplateAudioProcessor::valueTreePropertyChanged(juce::ValueTree&, 
    const juce::Identifier&) 
{
        parametersChanged.store(true);
}

template<typename T>
void BasicTemplateAudioProcessor::castParameter(
    juce::AudioProcessorValueTreeState& apvts, 
    const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination); // debug will show assertion failure if zero or null
}

bool BasicTemplateAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BasicTemplateAudioProcessor::createEditor()
{
    auto editor = new juce::GenericAudioProcessorEditor(*this);
    editor->setSize(500, 500);
    return editor;
}

void BasicTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void BasicTemplateAudioProcessor::setStateInformation (const void* data, 
    int sizeInBytes)
{
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicTemplateAudioProcessor();
}
