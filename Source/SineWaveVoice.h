#pragma once

#include "SineWaveSound.h"

class SineWaveVoice : public juce::SynthesiserVoice
{
public:
    enum class OscillatorMode
    {
        Sine,
        Triangle,
        Saw,
        Square,
        Noise
    };
    
    float coarseTune = 0.0f;
    float fineTune = 0.0f;
    
    SineWaveVoice(){
        // Initialize ADSR default parameters
        adsrParams.attack = 0.1f;
        adsrParams.decay = 0.1f;
        adsrParams.sustain = 0.7f;
        adsrParams.release = 0.1f;

        adsr.setParameters(adsrParams);  // Set the default ADSR values
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int) override
    {
        noteNumber = midiNoteNumber;
        currentAngle = 0.0;
        level = velocity;
        tailOff = 0.0;
        frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        updateFrequency();
        angleDelta = juce::MathConstants<double>::twoPi * frequency / getSampleRate();
        
        adsr.noteOn();
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
            {
                adsr.noteOff();
            }
            else
            {
                clearCurrentNote();
                adsr.reset();
            }
    }
    
    void setADSR(const juce::ADSR::Parameters& newParams)
    {
        adsrParams = newParams;
        adsr.setParameters(adsrParams);  // Apply the new ADSR settings
    }
    
    void setCoarseTune(float newValue)
    {
        coarseTune = newValue;
        updateFrequency();
    }
    
    void setFineTune(float newValue)
    {
        fineTune = newValue;
        updateFrequency();
    }
    
    void updateFrequency()
    {
        // Apply coarse (in semitones) and fine (fraction of a semitone) tuning
        double semitoneOffset = coarseTune + fineTune;
        double baseFrequency = juce::MidiMessage::getMidiNoteInHertz(noteNumber);
        double tunedFrequency = baseFrequency * std::pow(2.0, semitoneOffset / 12.0);
        angleDelta = juce::MathConstants<double>::twoPi * tunedFrequency / getSampleRate();

        // Log values to desktop log file
        juce::Logger::writeToLog("updateFrequency | coarseTune: " + juce::String(coarseTune) +
                                 ", fineTune: " + juce::String(fineTune) +
                                 ", tunedFrequency: " + juce::String(frequency));
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta == 0.0)
            return;

        auto numChannels = outputBuffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float sampleValue = 0.0f;

            switch (mode)
            {
                case OscillatorMode::Sine:
                    sampleValue = std::sin(currentAngle);
                    break;

                case OscillatorMode::Square:
                    sampleValue = std::sin(currentAngle) >= 0.0 ? 1.0f : -1.0f;
                    break;

                case OscillatorMode::Saw:
                    sampleValue = (float)(2.0 * (currentAngle / juce::MathConstants<double>::twoPi) - 1.0);
                    break;

                case OscillatorMode::Triangle:
                    sampleValue = (float)(2.0 * std::abs(2.0 * (currentAngle / juce::MathConstants<double>::twoPi) - 1.0) - 1.0);
                    break;

                case OscillatorMode::Noise:
                    sampleValue = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                    break;
            }

            sampleValue *= level * adsr.getNextSample();

            if (tailOff > 0.0)
            {
                sampleValue *= tailOff;
                tailOff *= 0.99;

                if (tailOff < 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }

            for (int channel = 0; channel < numChannels; ++channel)
                outputBuffer.addSample(channel, startSample + sample, sampleValue);

            currentAngle += angleDelta;

            if (currentAngle > juce::MathConstants<double>::twoPi)
                currentAngle -= juce::MathConstants<double>::twoPi;
            
            if (!adsr.isActive())
                {
                    clearCurrentNote();
                }
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void setMode(OscillatorMode newMode)
    {
        mode = newMode;
    }

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double level = 0.0;
    double frequency = 0.0;
    double tailOff = 0.0;
    
    int noteNumber = -1;

    OscillatorMode mode = OscillatorMode::Sine;

    juce::ADSR adsr; // ADSR object for the voice envelope
    juce::ADSR::Parameters adsrParams; // ADSR parameters that are controlled by the sliders
};
