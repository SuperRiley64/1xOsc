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
    apvts.addParameterListener("waveform", this); // <-- Add this line to listen for waveform changes
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("plugin_debug_log.txt");
    juce::Logger::setCurrentLogger(new juce::FileLogger(logFile, "JUCE Plugin Debug Log", 100000));
    juce::Logger::writeToLog("Logger initialized.");
}

_1xOscAudioProcessor::~_1xOscAudioProcessor()
{
    apvts.removeParameterListener("waveform", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout _1xOscAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "waveform", "Waveform",
        juce::StringArray{"Sine", "Triangle", "Saw", "Square", "Noise"}, 0));

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
            selectedMode = SineWaveVoice::OscillatorMode::Square;
        else if (newValue == 2)
            selectedMode = SineWaveVoice::OscillatorMode::Triangle;
        else if (newValue == 3)
            selectedMode = SineWaveVoice::OscillatorMode::Saw;
        else if (newValue == 4)
            selectedMode = SineWaveVoice::OscillatorMode::Noise;

        // Loop through all voices and set the new mode
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
                voice->setMode(selectedMode);
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

    // Set the sample rate for the synth
    synth.setCurrentPlaybackSampleRate(sampleRate);
    juce::Logger::writeToLog("Synth voice count: " + juce::String(synth.getNumVoices()));
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
}

void _1xOscAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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
