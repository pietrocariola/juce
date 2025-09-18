#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common.h"

// TODO: Quantidade de parametros
const long unsigned int NUM_PARAMS = 12;

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
    freq_low_ = 20.0f;
    freq_mid_1_ = 160.0f;
    freq_mid_2_ = 1000.0f;
    freq_high_ = 20000.0f;

    Q_low_ = 1.0f;
    Q_mid_1_ = 1.0f;
    Q_mid_2_ = 1.0f;
    Q_high_ = 1.0f;

    gain_low_ = 1.0f;
    gain_mid_1_ = 1.0f;
    gain_mid_2_ = 1.0f;
    gain_high_ = 1.0f;

    castParameter(apvts, ParamID::freq_low, freqLowParam);
    castParameter(apvts, ParamID::Q_low, qLowParam);
    castParameter(apvts, ParamID::gain_low, gainLowParam);

    castParameter(apvts, ParamID::freq_mid_1, freqMid1Param);
    castParameter(apvts, ParamID::Q_mid_1, qMid1Param);
    castParameter(apvts, ParamID::gain_mid_1, gainMid1Param);
    
    castParameter(apvts, ParamID::freq_mid_2, freqMid2Param);
    castParameter(apvts, ParamID::Q_mid_2, qMid2Param);
    castParameter(apvts, ParamID::gain_mid_2, gainMid2Param);

    castParameter(apvts, ParamID::freq_high, freqHighParam);
    castParameter(apvts, ParamID::Q_high, qHighParam);
    castParameter(apvts, ParamID::gain_high, gainHighParam);
    
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

    setCoeffs();
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
    smoother.setCurrentAndTargetValue(freqLowParam->get());
    freq_low_ = freqLowParam->get();

    smoother.setCurrentAndTargetValue(qLowParam->get());
    Q_low_ = qLowParam->get();

    smoother.setCurrentAndTargetValue(gainLowParam->get());
    gain_low_ = gainLowParam->get();

    smoother.setCurrentAndTargetValue(freqMid1Param->get());
    freq_mid_1_ = freqMid1Param->get();

    smoother.setCurrentAndTargetValue(qMid1Param->get());
    Q_mid_1_ = qMid1Param->get();

    smoother.setCurrentAndTargetValue(gainMid1Param->get());
    gain_mid_1_ = gainMid1Param->get();

    smoother.setCurrentAndTargetValue(freqMid2Param->get());
    freq_mid_2_ = freqMid2Param->get();

    smoother.setCurrentAndTargetValue(qMid2Param->get());
    Q_mid_2_ = qMid2Param->get();

    smoother.setCurrentAndTargetValue(gainMid2Param->get());
    gain_mid_2_ = gainMid2Param->get();

    smoother.setCurrentAndTargetValue(freqHighParam->get());
    freq_high_ = freqHighParam->get();

    smoother.setCurrentAndTargetValue(qHighParam->get());
    Q_high_ = qHighParam->get();

    smoother.setCurrentAndTargetValue(gainHighParam->get());
    gain_high_ = gainHighParam->get();

    setCoeffs();
}

// Configura os coeficientes do filtro
void MyAudioProcessor::setCoeffs() //AUDIO THREAD!!!
{
    lowShelfCoeff = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), freq_low_, Q_low_, gain_low_);
    midPeak1Coeff = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), freq_mid_1_, Q_mid_1_, gain_mid_1_);
    midPeak2Coeff = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), freq_mid_2_, Q_mid_2_, gain_mid_2_);
    highShelfCoeff = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), freq_high_, Q_high_, gain_high_);

    *filterChain.get<0>().coefficients = *lowShelfCoeff;
    *filterChain.get<1>().coefficients = *midPeak1Coeff;
    *filterChain.get<2>().coefficients = *midPeak2Coeff;
    *filterChain.get<3>().coefficients = *highShelfCoeff;
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
        ParamID::freq_low,
        "Low Shelf Frequency",
        20.0f,
        20000.0f, 
        20.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain_low,
        "Low Shelf Gain",
        -10.0f,
         10.0f, 
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q_low,
        "Low Shelf Q",
        0.1f,
        50.0f, 
        1.0f));

    // Mid peak 1
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::freq_mid_1,
        "Mid 1 Frequency",
        20.0f,
        20000.0f, 
        160.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain_mid_1,
        "Mid 1 Gain",
        -10.0f,
         10.0f,
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q_mid_1,
        "Mid 1 Q",
        0.1f,
        50.0f, 
        1.0f));

    // Mid peak 2
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::freq_mid_2,
        "Mid 2 Frequency",
        20.0f,
        20000.0f, 
        1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain_mid_2,
        "Mid 2 Gain",
        -10.0f,
         10.0f,
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q_mid_2,
        "Mid 2 Q",
        0.1f,
        50.0f, 
        1.0f));

    // High shelf
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::freq_high,
        "High Shelf Frequency",
        20.0f,
        20000.0f, 
        20000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain_high,
        "High Shelf Gain",
        -10.0f,
         10.0f,
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q_high,
        "High Shelf Q",
        0.1f,
        50.0f, 
        1.0f));

    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("default", {20.0f, 1.0f, 1.0f,
                                    160.0f, 1.0f, 1.0f,
                                    1000.0f, 1.0f, 1.0f,
                                    20000.0f, 1.0f, 1.0f}));
}

// TODO: Define preset atual
void MyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = index;
    
    juce::RangedAudioParameter *params[NUM_PARAMS] = {
        freqLowParam,
        qLowParam,
        gainLowParam,
        freqMid1Param,
        qMid1Param,
        gainMid1Param,
        freqMid2Param,
        qMid2Param,
        gainMid2Param,
        freqHighParam,
        qHighParam,
        gainHighParam
    };
    
    const Preset& preset = presets[(unsigned int)index];

    for (long unsigned int i = 0; i < NUM_PARAMS; ++i) 
    {
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));
    }
    
    reset();
}

//==============================================================================
