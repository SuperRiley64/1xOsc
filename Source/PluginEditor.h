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
    
    juce::Slider levelSlider;

    // ADSR labels
    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;
    
    juce::Label levelLabel;
    
    // Tuning Sliders
    juce::Slider coarseTuneSlider;
    juce::Slider fineTuneSlider;
    
    // Tuning Slider labels
    juce::Label coarseTuneLabel;
    juce::Label fineTuneLabel;
    
    juce::Label coarseTuneValueLabel;
    juce::Label fineTuneValueLabel;
    
    // Special slider
    juce::Slider specialSlider;
    juce::Label specialLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> specialAttachment;

    // ADSR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> coarseTuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fineTuneAttachment;
    
    // Filter variables
    juce::Slider filterCutoffSlider;
    juce::Slider filterResonanceSlider;
    juce::Label filterCutoffLabel;
    juce::Label filterResonanceLabel;

    juce::ComboBox filterTypeBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;
    
    juce::Slider filterAttackSlider, filterDecaySlider, filterSustainSlider, filterReleaseSlider;
    juce::Label filterAttackLabel, filterDecayLabel, filterSustainLabel, filterReleaseLabel;
    
    juce::Slider filterAmountSlider;
    juce::Label filterAmountLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterAmountAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        filterAttackAttachment, filterDecayAttachment, filterSustainAttachment, filterReleaseAttachment;
    
    // Declare the ADSR logic
    void sliderValueChanged(juce::Slider* slider) override;

    // Helper method to configure sliders and labels
    void addSliderWithLabel(juce::Slider& slider, juce::Label& label, const juce::String& name);
    
    juce::Image backgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_1xOscAudioProcessorEditor)
};
