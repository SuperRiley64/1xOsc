/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

class SineWaveSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SineWaveVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int) override
    {
        // Convert MIDI note to frequency
        currentAngle = 0.0;
        level = velocity;
        frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        angleDelta = juce::MathConstants<double>::twoPi * frequency / getSampleRate();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (!allowTailOff)
            clearCurrentNote();
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (!isVoiceActive())
            return;

        auto* buffer = outputBuffer.getWritePointer(0);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float sampleValue = std::sin(currentAngle) * level;
            buffer[startSample + sample] = sampleValue;

            currentAngle += angleDelta;
        }
    }
    
    void pitchWheelMoved (int newPitchWheelValue) override
        {
            // Implement what should happen when the pitch wheel is moved.
            // For example, adjust the pitch of the sine wave.
        }
    
    void controllerMoved (int controllerNumber, int newControllerValue) override
        {
            // Implement what should happen when the pitch wheel is moved.
            // For example, adjust the pitch of the sine wave.
        }
    

private:
    double currentAngle = 0.0, angleDelta = 0.0, frequency = 440.0, level = 0.0;
};

class _1xOscAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    _1xOscAudioProcessor();
    ~_1xOscAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_1xOscAudioProcessor)
};
