#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common.h"

// TODO: Quantidade de parametros
const long unsigned int NUM_PARAMS = 4;

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
    castParameter(apvts, ParamID::delayLength, delayLengthParam);
    castParameter(apvts, ParamID::dryMix, dryMixParam);
    castParameter(apvts, ParamID::wetMix, wetMixParam);
    castParameter(apvts, ParamID::feedback, feedbackParam);

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

    // Ponteiros para o buffer circular
    delayReadPosition_ = 0;
    delayWritePosition_ = 0;

    sampleRate_ = getSampleRate();

    // Aloca e zera o buffer de delay
    delayBufferLength_ = (int)(2.0 * sampleRate_); //delay maximo == 2s

    // Checa resultado da alocacao para evitar buffer de tamanho zero
    if(delayBufferLength_ < 1)
        delayBufferLength_ = 1;

    delayBuffer_.setSize(2, delayBufferLength_);
    delayBuffer_.clear();

    delayLengthSmoother.reset(sampleRate, 0.05);

    parametersChanged.store(true);
    reset();
}

// TODO: funcao que processa audio em loop - AUDIO THREAD!!!
void MyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //ignora mensagens MIDI
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    const int numSamples = buffer.getNumSamples();
    int dpr, dpw; // dpr = delay read pointer; dpw = delay write pointer
    
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
        dpr = delayReadPosition_;
        dpw = delayWritePosition_;

        for (int i = 0; i < numSamples; ++i)
        {
            const float in = channelData[i];
            float out = 0.0;

            // input + delayed samples
            out = (dryMix_ * in + wetMix_ * delayData[dpr]);

            // Store the current information in the delay buffer. delayData[dpr] is the delay sample we just read,
            // i.e. what came out of the buffer. delayData[dpw] is what we write to the buffer, i.e. what goes in
            delayData[dpw] = in + (delayData[dpr] * feedback_);

            if (++dpr >= delayBufferLength_)
                dpr = 0;
            if (++dpw >= delayBufferLength_)
                dpw = 0;

            // Store the output sample in the buffer, replacing the input
            channelData[i] = out;
        }
    }
    
    delayReadPosition_ = dpr;
    delayWritePosition_ = dpw;

    //variavel parametersChanged muda pelo evento valueTreePropertyChanged que executa na thread de UI
    bool expected = true;
    if (parametersChanged.compare_exchange_strong(expected, false)) {
        update();
    }
}

// chamada logo DEPOIS de processar
void MyAudioProcessor::releaseResources() {}

// TODO: atualiza parametros - AUDIO THREAD!!!
void MyAudioProcessor::update() {
    delayLengthSmoother.setCurrentAndTargetValue(delayLengthParam->get());
    
    delayLength_ = delayLengthSmoother.getNextValue();
    dryMix_ = dryMixParam->get();
    wetMix_ = wetMixParam->get();
    feedback_ = feedbackParam->get();
    
    // Use the sample rate to figure out what the delay position offset should be
    // (since it is specified in seconds, and we need to convert it to a number of samples)
    // This was moved here since delayLength_ can be changed in the UI
    //
    // dpr = read position from the delay buffer, e.g., delayed samples
    // dpr initially points to the start of the last block of size (delayLength_ * sampleRate) in the delay buffer.
    // e.g.: if (delayLength = 1, sampleRate = 44100, delayBufferLength_ = 88200) => dpr = 44100
    // this means that after reading (delayLength_ * sampleRate) samples from the input,
    // dpr will point to the begining of the delay buffer. From that point on it will read
    // delayed samples and include them in the output
    delayReadPosition_ = (int)(delayWritePosition_ - (delayLength_ * sampleRate_) + delayBufferLength_) % delayBufferLength_;
}

//==============================================================================
// Gestao de parametros
//------------------------------------------------------------------------------
// TODO: Cria parametros e adiciona em layout
juce::AudioProcessorValueTreeState::ParameterLayout MyAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::delayLength,
        "Delay Time (s)", 0.0f, 2.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::dryMix,
        "Dry", 0.0f, 1.0f, 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::wetMix,
        "Wet", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::feedback,
        "Feedback", 0.0f, 1.0f, 0.1f));
    
    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("short/no feedback", {0.50f, 1.00f, 0.7f, 0.00f}));
    presets.emplace_back(Preset("long + feedback",  {1.00f, 1.00f, 0.7f, 0.5f}));
}

// TODO: Define preset atual
void MyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = index;
    
    juce::RangedAudioParameter *params[NUM_PARAMS] = {
        delayLengthParam,
        dryMixParam,
        wetMixParam,
        feedbackParam
    };
    
    const Preset& preset = presets[(unsigned int)index];

    for (long unsigned int i = 0; i < NUM_PARAMS; ++i)
    {
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));
    }
    
    reset();
}
//==============================================================================
