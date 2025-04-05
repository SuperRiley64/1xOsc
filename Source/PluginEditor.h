/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class _1xOscAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    _1xOscAudioProcessorEditor (_1xOscAudioProcessor&);
    ~_1xOscAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    _1xOscAudioProcessor& audioProcessor;

    juce::Slider waveformSelector;
    juce::Label waveformLabel;
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        std::unique_ptr<SliderAttachment> waveformAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_1xOscAudioProcessorEditor)
};
