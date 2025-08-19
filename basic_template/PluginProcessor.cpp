/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// pietro: constructor
BasicTemplateAudioProcessor::BasicTemplateAudioProcessor()
/* pietro: if JucePlugin_PreferredChannelConfigurations is not defined, 
the constructor uses JUCE’s BusesProperties to tell the base 
AudioProcessor class what audio channels (buses) the plugin supports.*/
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     // pietro: MidiEffect does not need audio input/output
                     #if ! JucePlugin_IsMidiEffect
                      // pietro: Synth does not need audio input, only output
                      #if ! JucePlugin_IsSynth
                       // pietro: add audio input
                       // pietro: true means enabled by default
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       // pietro: add audio output
                       // pietro: true means enabled by default
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // pietro: empty constructor body
    /* pietro: 
        - member initialize list passes parameters before initialization
        - constructor body runs after initilization
    */
}

// pietro: destructor
BasicTemplateAudioProcessor::~BasicTemplateAudioProcessor()
{
}

// pietro: initial "const": juce::String returned by membere function can not be modified directly 
// pietro: trailing "const": makes the member function const and then can be called on const objects
// and the object is not modified insied it
const juce::String BasicTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

// pietro: accepts or not midi input
bool BasicTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput // pietro: set in cmake
    return true;
   #else
    return false;
   #endif
}

// pietro: requires or not midi output
bool BasicTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput // pietro: set in cmake
    return true;
   #else
    return false;
   #endif
}

// pietro: is midi effect or not
bool BasicTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect // pietro: set in cmake
    return true;
   #else
    return false;
   #endif
}

// pietro: how long (in seconds) does the plugin keeps outputing audio after the input seizes
/* pietro:
    A reverb might have a tail of several seconds.

    A delay might have a tail equal to its delay time.

    A simple gain plugin has no tail — output stops immediately when input stops.
*/
double BasicTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

// pietro: how many presets does the target plugin has
int BasicTemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

// pietro: index of the current preset starting from 0
int BasicTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

// pietro: loads and change preset parameters
void BasicTemplateAudioProcessor::setCurrentProgram (int index)
{
}

// pietro: reads preset name
const juce::String BasicTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

// pietro: changes the name of the preset in position "index"
void BasicTemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

// pietro: member function called before starting processing audio and
// when sampleRate or samplesPerBlock change
void BasicTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

// pietro: is used to clean memory
void BasicTemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

// pietro: returns TRUE if the buss layout is supported (compare input and output)
#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

// pietro: main member function where audio is processed
void BasicTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
}

//==============================================================================

// pietro: tells if plugin has graphic interface
bool BasicTemplateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

// pietro: initializes the graphic interface called editor
juce::AudioProcessorEditor* BasicTemplateAudioProcessor::createEditor()
{
    return new BasicTemplateAudioProcessorEditor (*this);
}

//==============================================================================
// pietro: gets and saves preset parameters
// pietro: its a good practice to use AudioProcessorValueTreeState for handling 
// parameters and XML to save them
void BasicTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

// pietro: loads and sets preset parameters from XML, for example 
void BasicTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicTemplateAudioProcessor();
}
