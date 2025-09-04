#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/*  Namespace where IDs are declared.
    Add all parameters that will be in the apvts.
    To add a parameter simply copies the line inside the 
  macro (#define/#undef) and change its name PARAMETER_ID(secondParameter).
    It avoids writing const juce::ParameterID gain("gain", 1)
  for each parameter and makes the parameterID equals to the variable name.
    Remember casting the parameters to each corresponding IDs 
  in the constructor.
*/
namespace ParamID {
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);
    PARAMETER_ID(gain)
    #undef PARAMETER_ID
}

class BasicTemplateAudioProcessor  : public juce::AudioProcessor,
  private juce::ValueTree::Listener
{
public:
    
    /*  Constructor:
          - Initializer list for AudioProcesor (BusesProperties) and apvts.
          - Cast parameters pointing to apvts.
          - Add listener to apvts state.
    */
    BasicTemplateAudioProcessor();  
    
    //  Destructor
    ~BasicTemplateAudioProcessor() override; 

    //  Returns the name of the plugin defined in the CMAKE file.
    const juce::String getName() const; 
 
    /*  Returns true if the plugin supports midi inputs. Values are 
    set in CMAKE file.
    */
    bool acceptsMidi() const override;

    //  Returns true if the plugin generates midi. Values are set in CMAKE file.
    bool producesMidi() const override; 
    
    /*  Returns true if the plugin interacts with midi. Values are 
    set in CMAKE file.
    */
    bool isMidiEffect() const override; 

   //   "If not defined" preprocessor directive set in cmake.
   #ifndef JucePlugin_PreferredChannelConfigurations
    /*  Host may call this member function to check if your plugin supports 
    the layout that host is using.
    */
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    /*  How long (in seconds) does the plugin keeps outputing audio after 
    the input seizes.
    */
    double getTailLengthSeconds() const override;

    //  Returns how many presets does the target plugin has.
    int getNumPrograms() override;

    //  Returns the index of the current preset.
    int getCurrentProgram() override;

    //  Changes the current preset.
    void setCurrentProgram (int index) override;

    //  Returns the name of the preset.
    const juce::String getProgramName (int index) override;
    
    //  Changes the name of the preset.
    void changeProgramName (int index, const juce::String& newName) override;

    /*  Member function that runs before playing audio and when 
    sampleRate or samplesPerBlock change. 
        It's used to setup the initial configurations and to get the sampleRate.
        Parameters:
          - sampleRate: number of samples/second set in the host (e.g. 44.1kHZ).
          - samplesPerBlock: maximum block size that the host will send 
        to the plugin (e.g. 128, 256, 512). Small block sizes process with 
        lower latency.
    */ 
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    /*  Most important member function where the data is processed.
        Parameters:
          - juce::AudioBuffer<float>& buffer: block of samples received from 
        the host.
          - juce::MidiBuffer& midiMessages: block of midi messages from the host.
        Write the output audio samples to this same buffer on the desired channel.
        Use samples and messages from these parameters to handle audio and midi 
      effects.
        If you will only handle audio (or midi) use juce::ignoreUnused(buffer) or 
      juce::ignoreUnused(midiMessages) to discard those inputs.
    */
    void processBlock (juce::AudioBuffer<float>& buffer, 
      juce::MidiBuffer& midiMessages) override;

    //  Member function to delete resources and free memory.
    void releaseResources() override;    

    //  Initializes a generic empty GUI.
    juce::AudioProcessorEditor* createEditor() override;

    //  Tells if plugin has graphic interface. True in our case.
    bool hasEditor() const override;          

    /*  Grab all the plugin's parameters and save. It's a good practice 
    to use ValueTrees.
    */
    void getStateInformation (juce::MemoryBlock& destData) override;

    //  Loads preset and sets parameters accordingly.
    void setStateInformation (const void* data, int sizeInBytes) override;

    //  Gain variable used in processBlock.
    float gain_;

    /*  Gain parameter that will point to gain in 
    AudioProcessorValueTreeState apvts.
    */
    juce::AudioParameterFloat* gainParam;    

private:

    /*  Wraps a ValueTree connecting parameters with the host.
        Initializer list:
          - AudioProcessor&	processorToConnectTo: pointer to the 
        AudioProcessor plugin.
          - UndoManager*	undoManagerToUse: used to handle undo and 
        redo options on the host (nullptr in our case).
          - const Identifier&	valueTreeType: string identifier designed 
        for accessing properties by name ("Parameters" in our case).
          - ParameterLayout	parameterLayout: member function where the 
        layout is created (createParameterLayout in our case).
    */
    juce::AudioProcessorValueTreeState apvts;

    /*  Member function called when any parameter changes. 
        It stores true in the variable parametersChanged.
        Member function parameters are not used. 
    */
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&);
      
    /*  Variable that is checked after all loops ended in processBlock.
        If parameters have changed, then parametersChanged is true and the 
      updateParameters member function is called. Afterwards, parametersChanged 
      is reset to false.
    */
    std::atomic<bool> parametersChanged {false};

    /*  Member function called at the end of each processBlock.
        Runs when parametersChanged is set to true.
        Update variables with the parameters' new values.
        No return.
    */
    void BasicTemplateAudioProcessor::updateParameters();

    /*  Makes destination to point to an AudioParameterFloat object stored 
      inside apvts.
        If T is not the correct type, dynamic_cast returns a nullptr.
    */
    template<typename T>
    void castParameter (juce::AudioProcessorValueTreeState& apvts, 
      const juce::ParameterID& id, T& destination);
    
    //  Creates a layout over the editor's GUI, stacking parameters vertically.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    //  Object to make smooth transactions when parameters change
    juce::LinearSmoothedValue<float> smoother;    

    /*  Macro that expands into:
          - non-copyable declaration avoiding unwanted copies of 
        BasicTemplateAudioProcessorEditor.
          - memory leak detector.    
    */    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicTemplateAudioProcessor)
};
