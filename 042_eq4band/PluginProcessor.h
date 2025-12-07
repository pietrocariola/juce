//==============================================================================
// PluginProcessor.h: definicao do PluginProcessor
//==============================================================================

#pragma once
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Woverloaded-virtual=1"
#endif

#define _USE_MATH_DEFINES

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <atomic>
#include <vector>
#include <cmath>
#include <functional>

#include "Preset.h"

// TODO: Namespace onde os parametros do plugin sao declarados
// Para adicionar um parametro, adicionar uma nova linha PARAMETER_ID(<nome_parametro>)
// dentro do bloco #define/#undef

namespace ParamID {
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);
    PARAMETER_ID(freq_low)
    PARAMETER_ID(gain_low)
    PARAMETER_ID(Q_low)
    PARAMETER_ID(freq_mid_1)
    PARAMETER_ID(gain_mid_1)
    PARAMETER_ID(Q_mid_1)
    PARAMETER_ID(freq_mid_2)
    PARAMETER_ID(gain_mid_2)
    PARAMETER_ID(Q_mid_2)
    PARAMETER_ID(freq_high)
    PARAMETER_ID(gain_high)
    PARAMETER_ID(Q_high)
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
    // Frequencia
    float freq_low_;
    float freq_mid_1_;
    float freq_mid_2_;
    float freq_high_;

    // Q
    float Q_low_;
    float Q_mid_1_;
    float Q_mid_2_;
    float Q_high_;

    // Ganho
    float gain_low_;
    float gain_mid_1_;
    float gain_mid_2_;
    float gain_high_;
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
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>> filterChainL;

    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>> filterChainR;

    juce::dsp::IIR::Coefficients<float>::Ptr lowShelfCoeff;
    juce::dsp::IIR::Coefficients<float>::Ptr midPeak1Coeff;
    juce::dsp::IIR::Coefficients<float>::Ptr midPeak2Coeff;
    juce::dsp::IIR::Coefficients<float>::Ptr highShelfCoeff;

    // Parametro para definir frequencia
    juce::AudioParameterFloat* freqLowParam;
    juce::AudioParameterFloat* freqMid1Param;
    juce::AudioParameterFloat* freqMid2Param;
    juce::AudioParameterFloat* freqHighParam;

    // Parametro para definir Q (largura de banda)
    juce::AudioParameterFloat* qLowParam;
    juce::AudioParameterFloat* qMid1Param;
    juce::AudioParameterFloat* qMid2Param;
    juce::AudioParameterFloat* qHighParam;

    juce::AudioParameterFloat* gainLowParam;
    juce::AudioParameterFloat* gainMid1Param;
    juce::AudioParameterFloat* gainMid2Param;
    juce::AudioParameterFloat* gainHighParam;

    // Suavizador de trocas de parametros
    juce::LinearSmoothedValue<float> smoother;

    // Define coeficientes para todos os filtros
    void setCoeffs();
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyAudioProcessor)
};
