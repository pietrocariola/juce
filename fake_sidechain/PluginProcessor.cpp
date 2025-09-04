/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// pietro: constructor
FakeSidechainAudioProcessor::FakeSidechainAudioProcessor()
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
FakeSidechainAudioProcessor::~FakeSidechainAudioProcessor()
{
}

// pietro: initial "const": juce::String returned by membere function can not be modified directly 
// pietro: trailing "const": makes the member function const and then can be called on const objects
// and the object is not modified insied it
const juce::String FakeSidechainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

// pietro: accepts or not midi input
bool FakeSidechainAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput // pietro: set in cmake
    return true;
   #else
    return false;
   #endif
}

// pietro: requires or not midi output
bool FakeSidechainAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput // pietro: set in cmake
    return true;
   #else
    return false;
   #endif
}

// pietro: is midi effect or not
bool FakeSidechainAudioProcessor::isMidiEffect() const
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
double FakeSidechainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

// pietro: how many presets does the target plugin has
int FakeSidechainAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

// pietro: index of the current preset starting from 0
int FakeSidechainAudioProcessor::getCurrentProgram()
{
    return 0;
}

// pietro: loads and change preset parameters
void FakeSidechainAudioProcessor::setCurrentProgram (int index)
{
}

// pietro: reads preset name
const juce::String FakeSidechainAudioProcessor::getProgramName (int index)
{
    return {};
}

// pietro: changes the name of the preset in position "index"
void FakeSidechainAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

int64_t FakeSidechainAudioProcessor::getBar()
{
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bar = pos->getBarCount())
                return *bar;                
}

// pietro: member function called before starting processing audio and
// when sampleRate or samplesPerBlock change
void FakeSidechainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    envelopeIndex_ = 0;   
    envelopeWindowSize_ = 48000;
}

// pietro: is used to clean memory
void FakeSidechainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

// pietro: returns TRUE if the buss layout is supported (compare input and output)
#ifndef JucePlugin_PreferredChannelConfigurations
bool FakeSidechainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
void FakeSidechainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{    
    juce::ignoreUnused(midiMessages);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = channelData[sample] * volumeEnvelope(envelopeIndex_, envelopeWindowSize_);
            if (++envelopeIndex_ >= envelopeWindowSize_){
                envelopeIndex_ = 0;
            }
        }
        barCounter_ = getBar();
        if (currentBar_ < barCounter_){
            std::cout << currentBar_ << std::endl;   
            currentBar_ += 1;
        }
    }
}

float FakeSidechainAudioProcessor::volumeEnvelope(int envelopeIndex, int envelopeWindowSize)
{
    if(envelopeIndex < envelopeWindowSize/2)
    {
        return 0;
    } else {
        return 1;
    }
}

//==============================================================================

// pietro: tells if plugin has graphic interface
bool FakeSidechainAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

// pietro: initializes the graphic interface called editor
juce::AudioProcessorEditor* FakeSidechainAudioProcessor::createEditor()
{
    return new FakeSidechainAudioProcessorEditor (*this);
}

//==============================================================================
// pietro: gets and saves preset parameters
// pietro: its a good practice to use AudioProcessorValueTreeState for handling 
// parameters and XML to save them
void FakeSidechainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

// pietro: loads and sets preset parameters from XML, for example 
void FakeSidechainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FakeSidechainAudioProcessor();
}
