#pragma once

//==============================================================================
// Common.h: funcoes comuns a todos os plugins
//==============================================================================

//==============================================================================
// Funcoes gerais
//------------------------------------------------------------------------------
// Cria plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MyAudioProcessor(); }

// Nome do plugin
const juce::String MyAudioProcessor::getName() const { return JucePlugin_Name; }

// Tamanho da cauda gerada pelo processamento do plugin
double MyAudioProcessor::getTailLengthSeconds() const { return 0.0; }

// Indica se plugin tem editor ou nao
bool MyAudioProcessor::hasEditor() const { return true; }

// Cria editor generico
juce::AudioProcessorEditor* MyAudioProcessor::createEditor() 
{ 
    auto editor = new juce::GenericAudioProcessorEditor(*this);
    editor->setSize(500, 500);
    return editor;
}
//==============================================================================

//==============================================================================
// Gestao de presets
//------------------------------------------------------------------------------
// Quantidade de presets
int MyAudioProcessor::getNumPrograms()
{
    return int(presets.size());
}

// Indice do preset atual
int MyAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

// Nome do preset
const juce::String MyAudioProcessor::getProgramName (int index)
{
    return {presets[(unsigned int)index].name};
}

// Altera nome do preset
void MyAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName); 
}

// Retorna configuracao atual, com presets, para host capaz de salvar configuracoes, como uma DAW
void MyAudioProcessor::getStateInformation (juce::MemoryBlock& destData) 
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

// Restaura configuracoes salvas
void MyAudioProcessor::setStateInformation (const void* data, int sizeInBytes) 
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
        parametersChanged.store(true);
    }
}
//==============================================================================

//==============================================================================
// Gestao de parametros
//------------------------------------------------------------------------------
// Evento de mudanca de parametro
void MyAudioProcessor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) {
        parametersChanged.store(true);
}

// Converte tipo de dados do parametro para tipo apropriado
template<typename T>
void MyAudioProcessor::castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID())); //uma vez que T é inferido, nao precisa passar <T>
    jassert(destination); // Somente em debug: vai falhar se parametro nao existir ou for de tipo incorreto
}
//==============================================================================

//==============================================================================
// Configuracoes de MIDI e barramentos (nao utilizado)
//------------------------------------------------------------------------------
bool MyAudioProcessor::acceptsMidi() const { return false; }
bool MyAudioProcessor::producesMidi() const { return false; }
bool MyAudioProcessor::isMidiEffect() const { return false; }
bool MyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
//==============================================================================
