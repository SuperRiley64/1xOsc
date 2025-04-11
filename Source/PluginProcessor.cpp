/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "SineWaveVoice.h"

#include "SineWaveSound.h"

//==============================================================================



_1xOscAudioProcessor::_1xOscAudioProcessor()
    : AudioProcessor (BusesProperties()
                      #if ! JucePlugin_IsMidiEffect
                       .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      #if JucePlugin_WantsMidiInput
                      .withInput ("Midi Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                      ),
      apvts(*this, nullptr, "Parameters", createParameterLayout()) // Initialize APVTS
{
    
    // Parameter listeners
    apvts.addParameterListener("waveform", this);
    apvts.addParameterListener("coarseTune", this);
    apvts.addParameterListener("fineTune", this);
    
    apvts.addParameterListener("attack", this);
    apvts.addParameterListener("decay", this);
    apvts.addParameterListener("sustain", this);
    apvts.addParameterListener("release", this);
    
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("plugin_debug_log.txt");
    juce::Logger::setCurrentLogger(new juce::FileLogger(logFile, "JUCE Plugin Debug Log", 100000));
    juce::Logger::writeToLog("Logger initialized.");
}

_1xOscAudioProcessor::~_1xOscAudioProcessor()
{
    apvts.removeParameterListener("waveform", this);
    apvts.removeParameterListener("coarseTune", this);
    apvts.removeParameterListener("fineTune", this);
    
    apvts.removeParameterListener("attack", this);
    apvts.removeParameterListener("decay", this);
    apvts.removeParameterListener("sustain", this);
    apvts.removeParameterListener("release", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout _1xOscAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Waveform
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "waveform", "Waveform",
        juce::StringArray{"Sine", "Triangle", "Saw", "Square", "Noise"}, 0));
    
    // ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.01f, 5.0f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.01f,5.0f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.01f, 5.0f), 0.1f));
    
    // Tuning
    params.push_back(std::make_unique<juce::AudioParameterFloat>("coarseTune", "Coarse Tune", juce::NormalisableRange<float>(-36.0f, 36.0f, 1.0), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fineTune", "Fine Tune", juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    
    // Filtering
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "filterType", "Filter Type",
        juce::StringArray { "Lowpass", "Bandpass", "Highpass" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterCutoff", "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f), 1000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterResonance", "Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 1.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterAttack", "Filter Attack", juce::NormalisableRange<float>(0.01f, 5.0f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterDecay", "Filter Decay", juce::NormalisableRange<float>(0.01f, 5.0f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterSustain", "Filter Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterRelease", "Filter Release", juce::NormalisableRange<float>(0.01f, 5.0f), 0.1f));
    
    return { params.begin(), params.end() };
}

//==============================================================================
void _1xOscAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::Logger::writeToLog("parameterChanged called: " + parameterID);
    if (parameterID == "waveform")
    {
        juce::Logger::writeToLog("Waveform changed: " + juce::String(newValue));
        SineWaveVoice::OscillatorMode selectedMode = SineWaveVoice::OscillatorMode::Sine;

        if (newValue == 0)
            selectedMode = SineWaveVoice::OscillatorMode::Sine;
        else if (newValue == 1)
            selectedMode = SineWaveVoice::OscillatorMode::Triangle;
        else if (newValue == 2)
            selectedMode = SineWaveVoice::OscillatorMode::Saw;
        else if (newValue == 3)
            selectedMode = SineWaveVoice::OscillatorMode::Square;
        else if (newValue == 4)
            selectedMode = SineWaveVoice::OscillatorMode::Noise;
        
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
                voice->setMode(selectedMode);
        }
    }
    else if (parameterID == "coarseTune")
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
                voice->setCoarseTune(newValue);
        }
    }
    else if (parameterID == "fineTune")
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
                voice->setFineTune(newValue);
        }
    }
    else if (parameterID == "attack" || parameterID == "decay" ||
             parameterID == "sustain" || parameterID == "release")
    {
        juce::ADSR::Parameters newParams;
        newParams.attack  = apvts.getRawParameterValue("attack")->load();
        newParams.decay   = apvts.getRawParameterValue("decay")->load();
        newParams.sustain = apvts.getRawParameterValue("sustain")->load();
        newParams.release = apvts.getRawParameterValue("release")->load();

        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
                voice->setADSR(newParams);
        }
    }
    
}


const juce::String _1xOscAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool _1xOscAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool _1xOscAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool _1xOscAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double _1xOscAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int _1xOscAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int _1xOscAudioProcessor::getCurrentProgram()
{
    return 0;
}

void _1xOscAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String _1xOscAudioProcessor::getProgramName (int index)
{
    return {};
}

void _1xOscAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void _1xOscAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int waveform = static_cast<int>(*apvts.getRawParameterValue("waveform"));
    // Clear any existing voices
    synth.clearVoices();

    // Add voices
    for (int i = 0; i < 8; ++i)
    {
        
        synth.addVoice(new SineWaveVoice());
    }

    // Clear and set the sound
    synth.clearSounds();
    synth.addSound(new SineWaveSound());  // You can keep this for now or update it to match your voice
    
    // Set the waveform of the synth
    SineWaveVoice::OscillatorMode selectedMode = SineWaveVoice::OscillatorMode::Sine;

    if (waveform == 0)
        selectedMode = SineWaveVoice::OscillatorMode::Sine;
    else if (waveform == 1)
        selectedMode = SineWaveVoice::OscillatorMode::Triangle;
    else if (waveform == 2)
        selectedMode = SineWaveVoice::OscillatorMode::Saw;
    else if (waveform == 3)
        selectedMode = SineWaveVoice::OscillatorMode::Square;
    else if (waveform == 4)
        selectedMode = SineWaveVoice::OscillatorMode::Noise;

    // Loop through all voices and set the new mode
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
            voice->setMode(selectedMode);
    }

    // Set the sample rate for the synth
    synth.setCurrentPlaybackSampleRate(sampleRate);
    juce::Logger::writeToLog("Synth voice count: " + juce::String(synth.getNumVoices()));
    
    // Set up the filter
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    filter.prepare(spec);
    filter.reset();
}

void _1xOscAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool _1xOscAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void _1xOscAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    // Apply filter DSP
    auto* cutoffParam = apvts.getRawParameterValue("filterCutoff");
    auto* resonanceParam = apvts.getRawParameterValue("filterResonance");
    auto* typeParam = apvts.getRawParameterValue("filterType");

    if (auto* filterParams = filter.state.get())
    {
        // Get the parameter values
        float cutoff = cutoffParam->load();
        float resonance = resonanceParam->load();
        int type = static_cast<int>(typeParam->load());

        // Set the filter parameters safely
        filterParams->setCutOffFrequency(getSampleRate(), cutoff, resonance);

        using FilterType = juce::dsp::StateVariableFilter::Parameters<float>::Type;
        switch (type)
        {
            case 0: filterParams->type = FilterType::lowPass; break;
            case 1: filterParams->type = FilterType::bandPass; break;
            case 2: filterParams->type = FilterType::highPass; break;
            default: break;
        }

        // Process the filter on the buffer
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        //filter.process(context);
    }

}

//==============================================================================
bool _1xOscAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* _1xOscAudioProcessor::createEditor()
{
    return new _1xOscAudioProcessorEditor (*this);
}

//==============================================================================
void _1xOscAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // Save the state of the APVTS
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void _1xOscAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    // Restore the state of the APVTS
        std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary(data, sizeInBytes));
        
        if (xmlState != nullptr)
            if (xmlState->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void _1xOscAudioProcessor::parameterValueChanged(int parameterIndex, float newValue){
    
}

void _1xOscAudioProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting){

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new _1xOscAudioProcessor();
}
