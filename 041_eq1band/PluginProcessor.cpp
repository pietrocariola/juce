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
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    //TODO: inicializacao dos parametros do plugin
    freq_ = 20000.0f;
    Q_ = 1.0f;
    gain_ = 1.0f;

    castParameter(apvts, ParamID::freq, freqParam);
    castParameter(apvts, ParamID::Q, qParam);
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
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (unsigned int)samplesPerBlock;
    spec.numChannels = (unsigned int)getTotalNumOutputChannels();

    filterChain.prepare(spec);

    // Configurando os coeficientes do filtro
    midPeakCoeff = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), freq_, Q_, gain_);
    *filterChain.get<0>().coefficients = *midPeakCoeff;
}

// TODO: funcao que processa audio em loop - AUDIO THREAD!!!
void MyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    // define uma flag especial para a CPU que trunca valores muito pequenos para zero
    // em vez de processa-los como numeros denormais
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filterChain.process(context);

    //valueTreePropertyChanged altera variavel parametersChanged quando algum parametro muda
    bool expected = true;
    if (parametersChanged.compare_exchange_strong(expected, false)) {
        update();
    }
}

// chamada logo DEPOIS de processar
void MyAudioProcessor::releaseResources() {}

// TODO: atualiza parametros - AUDIO THREAD!!!
void MyAudioProcessor::update() {
    smoother.setCurrentAndTargetValue(freqParam->get());
    freq_ = freqParam->get();

    smoother.setCurrentAndTargetValue(qParam->get());
    Q_ = qParam->get();

    smoother.setCurrentAndTargetValue(gainParam->get());
    gain_ = gainParam->get();

    *filterChain.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), freq_, Q_, gain_);
}

//==============================================================================
// Gestao de parametros
//------------------------------------------------------------------------------
// TODO: Cria parametros e adiciona em layout
juce::AudioProcessorValueTreeState::ParameterLayout MyAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::freq,
        "Cutoff Frequency",
        20.0f,
        20000.0f, 
        1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q,
        "Q",
        0.1f,
        100.0f, 
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain,
        "Gain",
        -100.0f,
        100.0f, 
        1.0f));


    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("20 kHz", {20000.0f, 1.0f, 1.0f}));
}

// TODO: Define preset atual
void MyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = index;
    
    juce::RangedAudioParameter *params[NUM_PARAMS] = {
        freqParam,
        qParam,
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
