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
    
    apvts.addParameterListener("filterAttack", this);
    apvts.addParameterListener("filterDecayRelease", this);
    apvts.addParameterListener("filterSustain", this);
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
    
    apvts.removeParameterListener("filterAttack", this);
    apvts.removeParameterListener("filterDecayRelease", this);
    apvts.removeParameterListener("filterSustain", this);
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
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "special", "Special", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    
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
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterDecayRelease", "Filter Decay/Release", juce::NormalisableRange<float>(0.01f, 5.0f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterSustain", "Filter Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterAmount", "Filter Amount",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f), 0.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "level", "Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));
    
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
    
    else if (parameterID == "filterAttack" || parameterID == "filterDecayRelease" ||
             parameterID == "filterSustain")
    {
        juce::ADSR::Parameters filterParams;
        filterParams.attack  = apvts.getRawParameterValue("filterAttack")->load();
        filterParams.decay   = apvts.getRawParameterValue("filterDecayRelease")->load();
        filterParams.sustain = apvts.getRawParameterValue("filterSustain")->load();
        filterParams.release = apvts.getRawParameterValue("filterDecayRelease")->load(); // same knob

        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
                voice->setFilterADSR(filterParams);
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

    for (auto& f : filters)
    {
        f.prepare(spec);
        f.reset();
    }
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
    
    // Update the special knob
    float special = apvts.getRawParameterValue("special")->load();
    // update the special value in the voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
        {
            if (voice->isVoiceActive())
            {
                voice->setSpecial(special);
            }
        }
    }
    
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    // Apply filter DSP
    auto* cutoffParam = apvts.getRawParameterValue("filterCutoff");
    auto* resonanceParam = apvts.getRawParameterValue("filterResonance");
    auto* typeParam = apvts.getRawParameterValue("filterType");
    
    float filterAmount = apvts.getRawParameterValue("filterAmount")->load() / 100.0f; // -1.0 to 1.0
    
    
    float baseCutoff = std::clamp(cutoffParam->load(), 20.0f, 20000.0f);
    float resonance = std::clamp(resonanceParam->load(), 0.1f, 10.0f);
    int typeValue = static_cast<int>(typeParam->load());

    juce::dsp::StateVariableFilter::Parameters<float>::Type filterType =
        juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass;
    if (typeValue == 1) filterType = juce::dsp::StateVariableFilter::Parameters<float>::Type::bandPass;
    else if (typeValue == 2) filterType = juce::dsp::StateVariableFilter::Parameters<float>::Type::highPass;

    // pass all that to each active voice
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
        {
            if (voice->isVoiceActive())
            {
                voice->setFilterParams(baseCutoff, resonance, filterType, filterAmount);
            }
        }
    }
    
    // Apply the level
    float level = *apvts.getRawParameterValue("level");
    buffer.applyGain(level);

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
