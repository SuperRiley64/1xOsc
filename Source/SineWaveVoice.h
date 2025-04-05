/*
  ==============================================================================

    SineWaveVoice.h
    Created: 3 Apr 2025 4:16:28pm
    Author:  Riley Knybel

  ==============================================================================
*/

#pragma once

#include "SineWaveSound.h"

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
        currentAngle = 0.0;
        level = velocity;
        tailOff = 0.0;
        frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        angleDelta = juce::MathConstants<double>::twoPi * frequency / getSampleRate();
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0; // start fading out
        }
        else
        {
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta == 0.0)
            return;

        auto numChannels = outputBuffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float sampleValue = std::sin(currentAngle) * level;

            if (tailOff > 0.0)
            {
                sampleValue *= tailOff;
                tailOff *= 0.99; // exponential fade-out

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
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double level = 0.0;
    double frequency = 0.0;
    double tailOff = 0.0;
};
