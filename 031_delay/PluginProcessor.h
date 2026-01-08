//==============================================================================
// PluginProcessor.h: definicao do PluginProcessor
//==============================================================================

#pragma once
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Woverloaded-virtual=1"
#endif

#define _USE_MATH_DEFINES

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <vector>
#include <cmath>

#include "Preset.h"

// TODO: Namespace onde os parametros do plugin sao declarados
// Para adicionar um parametro, adicionar uma nova linha PARAMETER_ID(<nome_parametro>)
// dentro do bloco #define/#undef

namespace ParamID {
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);
    PARAMETER_ID(delayLength) // Tamanho do delay em segundos
    PARAMETER_ID(dryMix)      // Nivel do sinal original (0-1)
    PARAMETER_ID(wetMix)      // Nivel do sinal atrasado (0-1)
    PARAMETER_ID(feedback)    // Nivel do feedback (0-quase 1)
    #undef PARAMETER_ID
}

class MyAudioProcessor : public juce::AudioProcessor, private juce::ValueTree::Listener
{
public:
    //==============================================================================
    // Construtor e destrutor
    //------------------------------------------------------------------------------
    MyAudioProcessor();
    ~MyAudioProcessor() override;
    //==============================================================================

    //==============================================================================
    // Funcoes de processamento de audio
    //------------------------------------------------------------------------------
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;
    double getTailLengthSeconds() const override;
    //==============================================================================

    //==============================================================================
    // Controle de GUI
    //------------------------------------------------------------------------------
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    //==============================================================================

    //==============================================================================
    // Gestao de presets
    //------------------------------------------------------------------------------
    const juce::String getName() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================

    //==============================================================================
    // Configuracoes de MIDI e barramentos (nao vamos usar)
    //------------------------------------------------------------------------------
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    //==============================================================================

    //==============================================================================
    // TODO: Parametros ajustaveis do plugin:
    //------------------------------------------------------------------------------
    float delayLength_;
    float dryMix_;
    float wetMix_;
    float feedback_;
private:
    //==============================================================================
    // Gestao de parametros
    //------------------------------------------------------------------------------
    // Arvore de parametros
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };
    // Lista de presets
    std::vector<Preset> presets;
    // Indice do preset atual
    int currentProgram;
    // Indica se algum parametro mudou
    std::atomic<bool> parametersChanged { false };    

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void update();
    void createPrograms();

    template<typename T>
    inline static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& id, T& destination);
    //==============================================================================

    //==============================================================================
    // TODO: Detalhes especificos deste plugin
    //------------------------------------------------------------------------------
    // Circular buffer variables for implementing delay
    juce::AudioBuffer<float> delayBuffer_;
    int delayBufferLength_;
    int delayReadPosition_, delayWritePosition_;

    double sampleRate_;

    // Suavizador de trocas de parametros
    juce::LinearSmoothedValue<float> delayLengthSmoother;

    juce::AudioParameterFloat* delayLengthParam;
    juce::AudioParameterFloat* dryMixParam;
    juce::AudioParameterFloat* wetMixParam;
    juce::AudioParameterFloat* feedbackParam;
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyAudioProcessor)
};
