#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common.h"

// TODO: Quantidade de parametros
const long unsigned int NUM_PARAMS = 2;

//==============================================================================
// Construtor e destrutor
//------------------------------------------------------------------------------
MyAudioProcessor::MyAudioProcessor()
    : AudioProcessor (BusesProperties()
        //TODO: Define se plugin mono ou stereo
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    //TODO: Inicializacao dos parametros do plugin
    gain_ = 1.0f;

    //tangente hiperbolica
    shapingFunctions.push_back([](float x) { return std::tanh(x); });

    //TODO: EXERCICIO - implementar as seguintes funcoes de saturacao e adiciona-las na lista de funcoes com shapingFunctions.push_back(<nova funcao>)

    //soft-clipping
    //return std::copysign(1.0, x) * (1 - 0.25 / (std::fabs(x) + 0.25))

    //hard-clipping
    //return std::max<float>(std::min<float>(x, 0.5), -0.5)

    // Primeira funcao da lista como default
    shapingFunc = shapingFunctions[0];

    castParameter(apvts, ParamID::gain, gainParam);
    castParameter(apvts, ParamID::waveshapingFunc, waveshapingFuncParam);
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

// TODO: Funcao que roda logo ANTES de começar a processar
void MyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
     juce::ignoreUnused(sampleRate, samplesPerBlock); 
}

// TODO: Funcao que processa audio em loop - AUDIO THREAD!!!
void MyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //ignora mensagens MIDI
    juce::ignoreUnused(midiMessages);

    // define uma flag especial para a CPU que trunca valores muito pequenos para zero
    // em vez de processa-los como numeros denormais
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // limpa buffer para canais alem dos em uso
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    //loop pelos canais (plugin stereo)
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        //ponteiro para canal
        auto* channelData = buffer.getWritePointer(channel);
        
        //loop pelas amostras de audio no buffer
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // altera audio direto no buffer do canal via ponteiro
            channelData[sample] = shape(gain_ * channelData[sample]);
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
    unsigned long i = static_cast<std::vector<int>::size_type>(waveshapingFuncParam->getIndex());
    shapingFunc = shapingFunctions[i];

    smoother.setCurrentAndTargetValue(gainParam->get());
    gain_ = gainParam->get();

    // exemplo de debug de parametro na console (precisa compilar em modo debug)  
    // std::stringstream ss;
    // ss << "gain: " << gain_;
    // DBG(ss.str()); //print na console para debug rapido
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

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        //nome do parametro
        ParamID::waveshapingFunc,
        //nome do parametro legivel
        "Waveshaping Function",
        //valores possiveis
        juce::StringArray { "tanh" }, //TODO: EXERCICIO - adicionar as strings "soft-clipping" e "hard-clipping"
        //indice default
        0));

    return layout;
}

//==============================================================================
// Gestao de presets (somente funcoes que variam por plugin. Ver Common.h)
//------------------------------------------------------------------------------

// TODO: Cria presets iniciais
void MyAudioProcessor::createPrograms()
{
    presets.emplace_back(Preset("Tanh shaping", {1.0f, 0}));
    
    //TODO: EXERCICIO - adicionar presets para as novas funcoes
    //presets.emplace_back(Preset("Soft-clipping distortion", {1.0f, 1}));
    //presets.emplace_back(Preset("Hard-clipping distortion", {1.0f, 2}));

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

//==============================================================================
// Detalhes especificos deste plugin
//------------------------------------------------------------------------------
// Funcao de waveshaping
float MyAudioProcessor::shape(float x)
{
    return shapingFunc(x);
}
//==============================================================================
