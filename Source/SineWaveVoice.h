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
    
    float filterEnvelopeValue = 0.0f;
    
    SineWaveVoice(){
        // Initialize ADSR default parameters
        adsrParams.attack = 0.1f;
        adsrParams.decay = 0.1f;
        adsrParams.sustain = 0.7f;
        adsrParams.release = 0.1f;

        adsr.setParameters(adsrParams);  // Set the default ADSR values
        
        filterEnvelopeParams.attack = 0.1f;
        filterEnvelopeParams.decay = 0.1f;  // You’ll use the combined decay/release here
        filterEnvelopeParams.sustain = 0.7f;
        filterEnvelopeParams.release = 0.1f;

        filterEnvelope.setParameters(filterEnvelopeParams);
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }
    
    void setCurrentPlaybackSampleRate(double newRate) override
    {
        SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
        filterEnvelope.setSampleRate(newRate);
        filter.prepare({ newRate, 512, 1 });
    }
    
    void setFilterParams(float cutoff, float resonance,
                         juce::dsp::StateVariableFilter::Parameters<float>::Type type,
                         float envAmount)
    {
        filterCutoff = cutoff;
        filterResonance = resonance;
        filterType = type;
        filterAmount = envAmount;
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
        
        adsr.noteOn();
        filterEnvelope.setParameters(filterEnvelopeParams);
        filterEnvelope.reset();
        filterEnvelope.noteOn();
        filter.prepare({ getSampleRate(), 512, 1 });
        
        // randomize the supersaw phases
        for (int i = 0; i < 7; ++i)
        {
            detunedPhases[i] = juce::Random::getSystemRandom().nextDouble() * juce::MathConstants<double>::twoPi;
        }
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
            {
                adsr.noteOff();
                filterEnvelope.noteOff();
            }
            else
            {
                adsr.reset();
                filterEnvelope.reset();
                clearCurrentNote();
            }
    }
    
    void setADSR(const juce::ADSR::Parameters& newParams)
    {
        adsrParams = newParams;
        adsr.setParameters(adsrParams);  // Apply the new ADSR settings
    }
    
    void setFilterADSR(const juce::ADSR::Parameters& newParams)
    {
        juce::Logger::writeToLog("setFilterADSR called: A=" + juce::String(newParams.attack) +
                                 ", D=" + juce::String(newParams.decay) +
                                 ", S=" + juce::String(newParams.sustain) +
                                 ", R=" + juce::String(newParams.release));
        filterEnvelopeParams = newParams;
        filterEnvelope.setParameters(filterEnvelopeParams);
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
    
    void setSpecial(float newValue)
    {
        special = newValue;
    }
    
    void updateFrequency()
    {
        // Apply coarse (in semitones) and fine (fraction of a semitone) tuning
        double semitoneOffset = coarseTune + fineTune;
        double baseFrequency = juce::MidiMessage::getMidiNoteInHertz(noteNumber);
        tunedFrequency = baseFrequency * std::pow(2.0, semitoneOffset / 12.0);
        angleDelta = juce::MathConstants<double>::twoPi * tunedFrequency / getSampleRate();

        // Log values to desktop log file
        juce::Logger::writeToLog("updateFrequency | coarseTune: " + juce::String(coarseTune) +
                                 ", fineTune: " + juce::String(fineTune) +
                                 ", tunedFrequency: " + juce::String(tunedFrequency));
    }
    
    float getFilterEnvelopeValue() const { return filterEnvelopeValue; }

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
                {
                    sampleValue = 0.0f;

                    // Dynamically compute number of harmonics based on 'special' (1 to 20)
                    int maxHarmonics = static_cast<int>(1 + special * 20);  // 1 to 50 harmonics

                    for (int h = 1; h <= maxHarmonics; ++h)
                    {
                        float amplitude = 1.0f / static_cast<float>(h);  // simple harmonic falloff
                        sampleValue += amplitude * std::sin(currentAngle * h);
                    }

                    sampleValue *= 0.5f; // basic normalization (can be tuned further if needed)
                    break;
                }
                case OscillatorMode::Triangle:
                {
                    // Desmos-based triangle with g = special
                    // Mimics Arturia Minibrute metalizer
                    double g =  special * 10 + 1;
                    double phase = fmod(currentAngle / juce::MathConstants<double>::twoPi, 1.0); // 0 to 1

                    double t = std::abs(fmod(phase, 1.0) - 0.5) * 2.0 * g;
                    double a = std::min(t, 1.0) - std::max(t, 1.0) + 1.0;
                    double b = std::max(a, 0.0) - std::min(a, 0.0);

                    sampleValue = (float)b - 0.5;
                    break;
                }
                case OscillatorMode::Saw:
                {
                    if (special > 0.0f) {
                        float detuneOffsets[7] = { -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f };
                        sampleValue = 0.0f;
                        
                        for (int i = 0; i < 7; ++i)
                        {
                            float detune = detuneOffsets[i] * static_cast<float>(special/2.0f); // bring the range down a bit
                            double freq = tunedFrequency * std::pow(2.0, detune / 12.0);
                            double phaseDelta = juce::MathConstants<double>::twoPi * freq / getSampleRate();
                            
                            detunedPhases[i] += phaseDelta;
                            if (detunedPhases[i] > juce::MathConstants<double>::twoPi)
                                detunedPhases[i] -= juce::MathConstants<double>::twoPi;
                            
                            double phase = detunedPhases[i] / juce::MathConstants<double>::twoPi;
                            double saw = 2.0 * phase - 1.0;
                            
                            sampleValue += static_cast<float>(saw);
                        }
                        
                        sampleValue /= 7.0f;
                        break;
                    }
                    else {
                        // proper simple saw with wrapped phase
                        double phase = fmod(currentAngle, juce::MathConstants<double>::twoPi) / juce::MathConstants<double>::twoPi;
                        sampleValue = static_cast<float>(2.0 * phase - 1.0);
                        break;
                    }
                }
                case OscillatorMode::Square:
                {
                    // Pulse Width Modulation — center = 0.5, range = 0.01 to 0.99
                    double pw = 0.5 + special * 0.49;
                    double phase = fmod(currentAngle / juce::MathConstants<double>::twoPi, 1.0);
                    sampleValue = (phase < pw) ? 1.0f : -1.0f;
                    break;
                }
                case OscillatorMode::Noise:
                {
                    sampleValue = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                    break;
                }
            }

            float ampEnv = adsr.getNextSample();
            filterEnvelopeValue = filterEnvelope.getNextSample();
            
            // Apply envelope to filter cutoff
            //float modulatedCutoff = filterCutoff * std::pow(2.0f, filterEnvelopeValue * filterAmount);
            float modulatedCutoff = filterCutoff + filterEnvelopeValue * (filterAmount * (20000.0f - 20.0f)); // full range
            modulatedCutoff = std::clamp(modulatedCutoff, 20.0f, 20000.0f);
            //juce::Logger::writeToLog("env: " + juce::String(filterEnvelopeValue) + " modulatedCutoff: " + juce::String(modulatedCutoff));
            
            if (auto* params = filter.state.get())
            {
                params->setCutOffFrequency(getSampleRate(), modulatedCutoff, filterResonance);
                params->type = filterType;
            }

            float temp = sampleValue;
            float* channelData[] = { &temp };

            juce::dsp::AudioBlock<float> block(channelData, 1, 1); // 1 channel, 1 sample
            juce::dsp::ProcessContextReplacing<float> context(block);
            filter.process(context);

            sampleValue = temp;
            
            float filtered = sampleValue;
            
            sampleValue = filtered * level * ampEnv;

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
    double special = 0.0;
    double tunedFrequency = 0.0;
    double detunedPhases[7] = {};
    
    int noteNumber = -1;

    OscillatorMode mode = OscillatorMode::Sine;

    juce::ADSR adsr; // ADSR object for the voice envelope
    juce::ADSR::Parameters adsrParams; // ADSR parameters that are controlled by the sliders
    
    juce::ADSR filterEnvelope;
    juce::ADSR::Parameters filterEnvelopeParams;
    
    juce::dsp::ProcessorDuplicator<
        juce::dsp::StateVariableFilter::Filter<float>,
        juce::dsp::StateVariableFilter::Parameters<float>
    > filter;
    
    float filterCutoff = 1000.0f;
    float filterResonance = 0.7f;
    float filterAmount = 0.0f;
    juce::dsp::StateVariableFilter::Parameters<float>::Type filterType =
        juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass;
};
