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

            sampleValue *= level;

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

    OscillatorMode mode = OscillatorMode::Sine;
};
