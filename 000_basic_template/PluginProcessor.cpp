#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common.h"

// TODO: Quantidade de parametros
const long unsigned int NUM_PARAMS = 1;

//==============================================================================
// Construtor e destrutor
//------------------------------------------------------------------------------
MyAudioProcessor::MyAudioProcessor()
    : AudioProcessor (BusesProperties()
        //TODO: Define se plugin mono ou stereo
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    //TODO: inicializacao dos parametros do plugin
    gain_ = 1.0f;

    castParameter(apvts, ParamID::gain, gainParam);
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
     juce::ignoreUnused(sampleRate, samplesPerBlock); 
}

// TODO: funcao que processa audio em loop - AUDIO THREAD!!!
void MyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //ignora mensagens MIDI
    juce::ignoreUnused(midiMessages);

    //loop pelos canais (plugin stereo)
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        //ponteiro para canal
        auto* channelData = buffer.getWritePointer(channel);
        
        //loop pelas amostras de audio no buffer
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= gain_;
        }
    }

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
    smoother.setCurrentAndTargetValue(gainParam->get());

    gain_ = gainParam->get();

    // exemplo de debug de parametro na console (precisa compilar em modo debug)  
    std::stringstream ss;
    ss << "gain: " << gain_;
    DBG(ss.str()); //print na console para debug rapido
}

//==============================================================================
// Gestao de parametros
//------------------------------------------------------------------------------
// TODO: Cria parametros e adiciona em layout
juce::AudioProcessorValueTreeState::ParameterLayout MyAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain,
        "Gain",
        juce::NormalisableRange(0.0f, 2.0f), 1.0f));

    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("unity gain", {1.0f}));
    presets.emplace_back(Preset("double gain", {2.0f}));
}

// TODO: Define preset atual
void MyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = index;
    
    juce::RangedAudioParameter *params[NUM_PARAMS] = {
        gainParam
    };

    const Preset& preset = presets[(unsigned int)index];

    for (long unsigned int i = 0; i < NUM_PARAMS; ++i)
    {
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));
    }
    
    reset();
}
//==============================================================================
