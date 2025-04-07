/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class _1xOscAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Slider::Listener
{
public:
    _1xOscAudioProcessorEditor (_1xOscAudioProcessor&);
    ~_1xOscAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    _1xOscAudioProcessor& audioProcessor;

    // Waveform selector
    juce::Label waveformLabel;
    juce::ComboBox waveformComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    // ADSR sliders
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    // ADSR labels
    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;

    // ADSR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    
    // Declare the ADSR logic
    void sliderValueChanged(juce::Slider* slider) override;
    void updateADSR();

    // Helper method to configure sliders and labels
    void addSliderWithLabel(juce::Slider& slider, juce::Label& label, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_1xOscAudioProcessorEditor)
};
