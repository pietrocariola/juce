#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common.h"

// TODO: Quantidade de parametros
const long unsigned int NUM_PARAMS = 3;

//==============================================================================
// Construtor e destrutor
//------------------------------------------------------------------------------
MyAudioProcessor::MyAudioProcessor()
    : AudioProcessor (BusesProperties()
        //TODO: Define se plugin mono ou stereo
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
        delayBuffer_(2, 1)
{
    //TODO: inicializacao dos parametros do plugin
    interpolation_ = Interpolation::Linear;
    
    // Set default values:
    sweepWidth_ = 0.001f;
    frequency_ = 2.0f;
        
    castParameter(apvts, ParamID::frequency, frequencyParam);
    castParameter(apvts, ParamID::sweepWidth, sweepWidthParam);
    castParameter(apvts, ParamID::interpolationType, interpolationTypeParam);

    apvts.state.addListener(this);
    
    createPrograms();
    setCurrentProgram(0);
}

MyAudioProcessor::~MyAudioProcessor() 
{
    apvts.state.removeListener(this);
}
//==============================================================================

//==============================================================================
// Funcoes de processamento de audio
//------------------------------------------------------------------------------

// TODO: funcao que roda logo ANTES de começar a processar
void MyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(samplesPerBlock); 

    sampleRate_ = (float)getSampleRate();
    
    delayBufferLength_ = 1;
    lfoPhase_ = 0.0;
    inverseSampleRate_ = 1.0f/sampleRate_;
    
    // Start the circular buffer pointer at the beginning
    delayWritePosition_ = 0;

    delayBufferLength_ = (int)(0.05*sampleRate) + 3;
    delayBuffer_.setSize(2, delayBufferLength_);
    delayBuffer_.clear();

    frequencySmoother.reset(sampleRate_, 0.05);
    
    lfoPhase_ = 0.0;
    inverseSampleRate_ = 1.0f/sampleRate_;
    
    parametersChanged.store(true);
    reset();
}

// TODO: funcao que processa audio em loop - AUDIO THREAD!!!
void MyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
   //ignora mensagens MIDI
    juce::ignoreUnused(midiMessages);
 
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    const int numSamples = buffer.getNumSamples();
    int dpw = 0;
    float dpr = 0;
    float currentDelay = 0;
    float ph = 0;
    
    // clears any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's audio processing...
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // get read/write pointer to buffer
        auto* channelData = buffer.getWritePointer(channel);

        // delayData is the circular buffer for implementing delay on this channel
        float* delayData = delayBuffer_.getWritePointer(juce::jmin(channel, delayBuffer_.getNumChannels() - 1));

        // Make a temporary copy of any state variables declared in PluginProcessor.h which need to be
        // maintained between calls to processBlock(). Each channel needs to be processed identically
        // which means that the activity of processing one channel can't affect the state variable for
        // the next channel.
        dpw = delayWritePosition_;
        ph = lfoPhase_;

        for (int i = 0; i < numSamples; ++i)
        {
            const float in = channelData[i];
            float interpolatedSample = 0.0f;
            
            currentDelay = sweepWidth_ * lfo(ph);
            
            // Subtract 3 samples to the delay pointer to make sure we have enough previously written
            // samples to interpolate with
            dpr = fmodf((float)dpw - (float)(currentDelay * sampleRate_) + (float)delayBufferLength_ - 3.0f,
                        (float)delayBufferLength_);
            
            if (interpolation_ == Interpolation::Linear)
            {
                // Find the fraction by which the read pointer sits between two
                // samples and use this to adjust weights of the samples
                float fraction = dpr - floorf(dpr);
                int previousSample = (int)floorf(dpr);
                int nextSample = (previousSample + 1) % delayBufferLength_;
                interpolatedSample = fraction * delayData[nextSample] + (1.0f - fraction) * delayData[previousSample];
            }
            else if (interpolation_ == Interpolation::Cubic)
            {
                // Cubic interpolation will produce cleaner results at the expense
                // of more computation. This code uses the Catmull-Rom variant of
                // cubic interpolation. To reduce the load, calculate a few quantities
                // in advance that will be used several times in the equation:
                int sample1 = (int)floorf(dpr);
                int sample2 = (sample1 + 1) % delayBufferLength_;
                int sample3 = (sample2 + 1) % delayBufferLength_;
                int sample0 = (sample1 - 1 + delayBufferLength_) % delayBufferLength_;
                
                float fraction = dpr - floorf(dpr);
                float frsq = fraction * fraction;
                
                float a0 = -0.5f*delayData[sample0] + 1.5f*delayData[sample1] - 1.5f*delayData[sample2] + 0.5f*delayData[sample3];
                float a1 = delayData[sample0] - 2.5f*delayData[sample1] + 2.0f*delayData[sample2] - 0.5f*delayData[sample3];
                float a2 = -0.5f*delayData[sample0] + 0.5f*delayData[sample2];
                float a3 = delayData[sample1];
                
                interpolatedSample = a0 * fraction * frsq + a1 * frsq + a2 * fraction + a3;
            }
            else // Nearest neighbour interpolation
            {
                // Find the nearest input sample by rounding the fractional index to the
                // nearest integer. It's possible this will round it to the end of the buffer,
                // in which case we need to roll it back to the beginning.
                int closestSample = (int)floorf(dpr + 0.5f);
                if(closestSample == delayBufferLength_)
                    closestSample = 0;
                interpolatedSample = delayData[closestSample];
            }

            // Store the current information in the delay buffer. With feedback, what we read is
            // included in what gets stored in the buffer, otherwise it's just a simple delay line
            // of the input signal.
            delayData[dpw] = in;

            // Increment the write pointer at a constant rate. The read pointer will move at different
            // rates depending on the settings of the LFO, the delay and the sweep width.
            
            if (++dpw >= delayBufferLength_)
                dpw = 0;

            // Store the output sample in the buffer, replacing the input. In the vibrato effect,
            // the delaye sample is the only component of the output (no mixing with the dry signal)
            channelData[i] = interpolatedSample;

            // Update the LFO phase, keeping it in the range 0-1
            ph += frequency_ * inverseSampleRate_;
            if(ph >= 1.0f)
                ph -= 1.0f;
        }
    }
    
    // Having made a local copy of the state variables for each channel, now transfer the result
    // back to the main state variable so they will be preserved for the next call of processBlock()
    delayWritePosition_ = dpw;
    lfoPhase_ = ph;

    //variable parametersChanged is updated by valueTreePropertyChanged running on any UI thread
    bool expected = true;
    if (parametersChanged.compare_exchange_strong(expected, false)) {
        update();
    }
}

//==============================================================================
// Function for calculating LFO waveforms. Phase runs from 0-1, output is scaled
// from 0 to 1 (note: not -1 to 1 as would be typical of sine).
float MyAudioProcessor::lfo(float phase)
{
    return 0.5f + 0.5f * sinf(2.0f * (float)M_PI * phase);
}

// chamada logo DEPOIS de processar
void MyAudioProcessor::releaseResources() {}

// TODO: atualiza parametros - AUDIO THREAD!!!
void MyAudioProcessor::update() {
    frequencySmoother.setCurrentAndTargetValue(frequencyParam->get());
    
    frequency_ = frequencySmoother.getNextValue();
    sweepWidth_ = sweepWidthParam->get();
    interpolation_ = static_cast<Interpolation>(interpolationTypeParam->getIndex());
}

//==============================================================================
// Gestao de parametros
//------------------------------------------------------------------------------
// TODO: Cria parametros e adiciona em layout
juce::AudioProcessorValueTreeState::ParameterLayout MyAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamID::frequency,
                                                           "Frequency (Hz)", 0.0f, 20.0f, 6.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamID::sweepWidth,
                                                           "Sweep Width",
                                                           juce::NormalisableRange(0.001f, 0.05f, 0.0005f),
                                                           0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParamID::interpolationType,
                                                            "Interpolation",
                                                            juce::StringArray { "Nearest Neighbor", "Linear", "Cubic" },
                                                            0));

    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("angry violin", {6.0f, 0.001f, 0}));
}

// TODO: Define preset atual
void MyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = index;
    
    juce::RangedAudioParameter *params[NUM_PARAMS] = {
        frequencyParam,
        sweepWidthParam,
        interpolationTypeParam
    };
    
    const Preset& preset = presets[(unsigned int)index];

    for (long unsigned int i = 0; i < NUM_PARAMS; ++i) 
    {
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));
    }
    
    reset();
}

//==============================================================================
