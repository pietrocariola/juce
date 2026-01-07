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
    wet_dry_mix_ = 0.5f; //50%

    castParameter(apvts, ParamID::wet_dry, wetDryMixParam);
    
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
void MyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) 
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (unsigned int)samplesPerBlock;
    spec.numChannels = (unsigned int)getTotalNumInputChannels();

    convolution.reset();
    convolution.prepare(spec);
    
    mixer.prepare(spec);
    mixer.setMixingRule(juce::dsp::DryWetMixingRule::balanced);
    mixer.setWetMixProportion(wet_dry_mix_);
}

// TODO: funcao que processa audio em loop - AUDIO THREAD!!!
void MyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    // define uma flag especial para a CPU que trunca valores muito pequenos para zero
    // em vez de processa-los como numeros denormais
    juce::ScopedNoDenormals noDenormals;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    mixer.pushDrySamples(block);
    convolution.process(context);
    mixer.mixWetSamples(block);

    //valueTreePropertyChanged altera variavel parametersChanged quando algum parametro muda
    bool expected = true;
    if (parametersChanged.compare_exchange_strong(expected, false)) {
        update();
    }
}

// chamada logo DEPOIS de processar
void MyAudioProcessor::releaseResources() {}

// TODO: atualiza parametros - AUDIO THREAD!!!
void MyAudioProcessor::update() 
{
    gainSmoother.setCurrentAndTargetValue(wetDryMixParam->get());
    wet_dry_mix_ = wetDryMixParam->get();

    mixer.setWetMixProportion(wet_dry_mix_);
}

void MyAudioProcessor::loadImpulseResponse(juce::File file)
{
    DBG("load file" << file.getFileName());
    convolution.loadImpulseResponse(file, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0);
}

//==============================================================================
// Gestao de parametros
//------------------------------------------------------------------------------
// TODO: Cria parametros e adiciona em layout
juce::AudioProcessorValueTreeState::ParameterLayout MyAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Low Shelf
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::wet_dry,
        "Dry/Wet Mix",
        0.0f,
        1.0f, 
        0.5f));

    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("default", {0.0f}));
}

// TODO: Define preset atual
void MyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = index;
    
    juce::RangedAudioParameter *params[NUM_PARAMS] = {
        wetDryMixParam
    };

    const Preset& preset = presets[(unsigned int)index];

    for (long unsigned int i = 0; i < NUM_PARAMS; ++i) 
    {
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));
    }
    
    reset();
}

//==============================================================================
