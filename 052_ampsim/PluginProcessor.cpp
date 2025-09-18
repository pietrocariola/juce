#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common.h"
#include "IR.h"

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
    freq_low_ = 50.0f;
    freq_mid_ = 450.0f;
    freq_high_ = 3000.0f;

    Q_low_ = 1.0f;
    Q_mid_ = 1.0f;
    Q_high_ = 1.0f;

    gain_low_ = 1.0f;
    gain_mid_ = 1.0f;
    gain_high_ = 1.0f;

    pre_gain_ = 0.0f;
    post_gain_ = 0.0f;

    irIndex = 0;

    castParameter(apvts, ParamID::freq_low, freqLowParam);
    castParameter(apvts, ParamID::Q_low, qLowParam);
    castParameter(apvts, ParamID::gain_low, gainLowParam);

    castParameter(apvts, ParamID::freq_mid, freqMidParam);
    castParameter(apvts, ParamID::Q_mid, qMidParam);
    castParameter(apvts, ParamID::gain_mid, gainMidParam);

    castParameter(apvts, ParamID::freq_high, freqHighParam);
    castParameter(apvts, ParamID::Q_high, qHighParam);
    castParameter(apvts, ParamID::gain_high, gainHighParam);
    
    castParameter(apvts, ParamID::pre_gain, preGainParam);
    castParameter(apvts, ParamID::post_gain, postGainParam);

    castParameter(apvts, ParamID::ir, irParam);

    auto& waveshaper = filterChain.get<4>();
    waveshaper.functionToUse = [](float x) { return (float)(std::copysign(1.0, x) * (1 - 0.25 / (std::fabs(x) + 0.25))); };

    prepareIR();

    apvts.state.addListener(this);
    
    createPrograms();
    setCurrentProgram(0);
}

MyAudioProcessor::~MyAudioProcessor() 
{
    apvts.state.removeListener(this);
}

void MyAudioProcessor::prepareIR()
{
    juce::WavAudioFormat format;

    for (int i = 0; i < 3; i++)
    {
        // tem que ser no heap porque o reader vai destrui-lo depois do uso
        juce::MemoryInputStream* inputStream = new juce::MemoryInputStream(IR[i], IR_BYTES, false);
        // "true" no final indica que reader deve destruir o stream no final
        // unique_ptr cuida de destruir o reader no final do escopo
        std::unique_ptr<juce::AudioFormatReader> reader = std::unique_ptr<juce::AudioFormatReader>(format.createReaderFor(inputStream, true));

        if (reader != nullptr)
        {
            long int samples = reader->lengthInSamples;
            IRBlock[i].allocate(samples, true);
            *IRBlock[i] = juce::AudioBuffer<float>(1, samples);
            reader->read(IRBlock[i], 0, samples, 0, true, true);
        }
    }
}

void MyAudioProcessor::loadIR()
{
    auto& convolution = filterChain.get<6>();
    convolution.loadImpulseResponse(IRBlock[irIndex], IR_SIZE, juce::dsp::Convolution::Stereo::no, juce::dsp::Convolution::Trim::yes, 0);
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

    loadIR();

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

    expected = true;
    if (irChanged.compare_exchange_strong(expected, false)) {
        loadIR();
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

    smoother.setCurrentAndTargetValue(freqMidParam->get());
    freq_mid_ = freqMidParam->get();

    smoother.setCurrentAndTargetValue(qMidParam->get());
    Q_mid_ = qMidParam->get();

    smoother.setCurrentAndTargetValue(freqHighParam->get());
    freq_high_ = freqHighParam->get();

    smoother.setCurrentAndTargetValue(qHighParam->get());
    Q_high_ = qHighParam->get();

    smoother.setCurrentAndTargetValue(gainLowParam->get());
    gain_low_ =  juce::Decibels::decibelsToGain(gainLowParam->get());

    smoother.setCurrentAndTargetValue(gainMidParam->get());
    gain_mid_ = juce::Decibels::decibelsToGain(gainMidParam->get());

    smoother.setCurrentAndTargetValue(gainHighParam->get());
    gain_high_ = juce::Decibels::decibelsToGain(gainHighParam->get());

    smoother.setCurrentAndTargetValue(preGainParam->get());
    pre_gain_ = preGainParam->get();

    smoother.setCurrentAndTargetValue(postGainParam->get());
    post_gain_ = postGainParam->get();

    unsigned int newIr = (unsigned int)irParam->getIndex();

    if (irIndex != newIr)
    {
        irIndex = newIr;
        irChanged.store(true);
    }

    setCoeffs();
}

// Configura os coeficientes do filtro
void MyAudioProcessor::setCoeffs() //AUDIO THREAD!!!
{
    lowShelfCoeff = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), freq_low_, Q_low_, gain_low_);
    midPeakCoeff = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), freq_mid_, Q_mid_, gain_mid_);
    highShelfCoeff = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), freq_high_, Q_high_, gain_high_);

    *filterChain.get<0>().coefficients = *lowShelfCoeff;
    *filterChain.get<1>().coefficients = *midPeakCoeff;
    *filterChain.get<2>().coefficients = *highShelfCoeff;

    auto& preGain = filterChain.get<3>();
    preGain.setGainDecibels(pre_gain_);
 
    auto& postGain = filterChain.get<5>();
    postGain.setGainDecibels(post_gain_);
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
        "Bass Frequency",
        20.0f,
        200.0f, 
        50.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain_low,
        "Bass Gain",
        juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q_low,
        "Bass Q",
        0.1f,
        50.0f, 
        1.0f));

    // Mid peak
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::freq_mid,
        "Mid Frequency",
        100.0f,
        600.0f, 
        450.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain_mid,
        "Mid Gain",
        juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q_mid,
        "Mid Q",
        0.1f,
        50.0f, 
        1.0f));

    // High shelf
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::freq_high,
        "Treble Frequency",
        1000.0f,
        5000.0f, 
        3000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::gain_high,
        "Treble Gain",
        juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::Q_high,
        "Treble Q",
        0.1f,
        50.0f, 
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::pre_gain,
        "Pre Gain",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamID::post_gain,
        "Post Gain",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f), 
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParamID::ir,
        "IR",
        juce::StringArray { "TwinVerb212", "GB412", "JZ120" }, 
        0));

    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("default", {50.0f, 1.0f, 1.0f,
                                    450.0f, 1.0f, 1.0f,
                                    3000.0f, 1.0f, 1.0f,
                                    0.0f, 0.0f, 0.0f}));

}

// TODO: Define preset atual
void MyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = index;
    
    juce::RangedAudioParameter *params[NUM_PARAMS] = {
        freqLowParam,
        qLowParam,
        gainLowParam,
        freqMidParam,
        qMidParam,
        gainMidParam,
        freqHighParam,
        qHighParam,
        gainHighParam,
        preGainParam,
        postGainParam,
        irParam
    };
    
    const Preset& preset = presets[(unsigned int)index];

    for (long unsigned int i = 0; i < NUM_PARAMS; ++i) 
    {
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));
    }
    
    reset();
}

//==============================================================================
