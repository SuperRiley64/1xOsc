/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SineWaveVoice.h"

//==============================================================================
_1xOscAudioProcessorEditor::_1xOscAudioProcessorEditor (_1xOscAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 300);

    // Label for the waveform selector
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(waveformLabel);
    
    waveformComboBox.addItem("Sine", 1);
    waveformComboBox.addItem("Triangle", 2);
    waveformComboBox.addItem("Saw", 3);
    waveformComboBox.addItem("Square", 4);
    waveformComboBox.addItem("Noise", 5);
    addAndMakeVisible(waveformComboBox);

    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.attachToComponent(&waveformComboBox, false);
    addAndMakeVisible(waveformLabel);

    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "waveform", waveformComboBox);
    
    // ADSR Sliders and Labels
    addSliderWithLabel(attackSlider, attackLabel, "Attack");
    addSliderWithLabel(decaySlider, decayLabel, "Decay");
    addSliderWithLabel(sustainSlider, sustainLabel, "Sustain");
    addSliderWithLabel(releaseSlider, releaseLabel, "Release");
    
    attackAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "attack",  attackSlider);
    decayAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "decay",   decaySlider);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "sustain", sustainSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "release", releaseSlider);
    
    addSliderWithLabel(coarseTuneSlider, coarseTuneLabel, "Coarse");
    addSliderWithLabel(fineTuneSlider, fineTuneLabel, "Fine");
    
    coarseTuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "coarseTune", coarseTuneSlider);

    fineTuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "fineTune", fineTuneSlider);
    
    // Coarse tune value label
    coarseTuneValueLabel.setJustificationType(juce::Justification::centred);
    coarseTuneValueLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(coarseTuneValueLabel);

    // Fine tune value label
    fineTuneValueLabel.setJustificationType(juce::Justification::centred);
    fineTuneValueLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(fineTuneValueLabel);
    
    coarseTuneValueLabel.setText(juce::String(coarseTuneSlider.getValue(), 0), juce::dontSendNotification);
    fineTuneValueLabel.setText(juce::String(fineTuneSlider.getValue(), 2), juce::dontSendNotification);
    
    coarseTuneSlider.onValueChange = [this]
    {
        coarseTuneValueLabel.setText(juce::String(coarseTuneSlider.getValue(), 0), juce::dontSendNotification);
    };

    fineTuneSlider.onValueChange = [this]
    {
        fineTuneValueLabel.setText(juce::String(fineTuneSlider.getValue(), 2), juce::dontSendNotification);
    };
    
    // Filter
    addSliderWithLabel(filterCutoffSlider, filterCutoffLabel, "Cutoff");
    addSliderWithLabel(filterResonanceSlider, filterResonanceLabel, "Resonance");

    filterTypeBox.addItem("Lowpass", 1);
    filterTypeBox.addItem("Bandpass", 2);
    filterTypeBox.addItem("Highpass", 3);
    addAndMakeVisible(filterTypeBox);

    // Attachments
    filterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filterCutoff", filterCutoffSlider);

    filterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filterResonance", filterResonanceSlider);

    filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "filterType", filterTypeBox);
    
    addSliderWithLabel(filterAttackSlider, filterAttackLabel, "F-Attack");
    addSliderWithLabel(filterDecaySlider, filterDecayLabel, "F-Decay");
    addSliderWithLabel(filterSustainSlider, filterSustainLabel, "F-Sustain");
    addSliderWithLabel(filterReleaseSlider, filterReleaseLabel, "F-Release");

    filterAttackAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "filterAttack",  filterAttackSlider);
    filterDecayAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "filterDecay",   filterDecaySlider);
    filterSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "filterSustain", filterSustainSlider);
    filterReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "filterRelease", filterReleaseSlider);
    
}

_1xOscAudioProcessorEditor::~_1xOscAudioProcessorEditor()
{
}

//==============================================================================
void _1xOscAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void _1xOscAudioProcessorEditor::addSliderWithLabel(juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::Rotary);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);
}

void _1xOscAudioProcessorEditor::resized()
{
    // Position waveformComboBox in the top-left corner
    waveformComboBox.setBounds(10, 40, 100, 30); // Adjust size and position as needed
    waveformLabel.setBounds(10, 70, 100, 20); // Position label below the combo box
    
    const int sliderSize = 70;
    const int adsrYOffset = 100;
    attackSlider.setBounds(20, adsrYOffset, sliderSize, sliderSize);
    decaySlider.setBounds(90, adsrYOffset, sliderSize, sliderSize);
    sustainSlider.setBounds(160, adsrYOffset, sliderSize, sliderSize);
    releaseSlider.setBounds(230, adsrYOffset, sliderSize, sliderSize);
    
    // Position sliders
    coarseTuneSlider.setBounds(120, 20, sliderSize, sliderSize);
    coarseTuneLabel.setBounds(120, 95, 75, 20);

    fineTuneSlider.setBounds(200, 20, sliderSize, sliderSize);
    fineTuneLabel.setBounds(200, 95, 75, 20);
    fineTuneLabel.setBounds(200, 95, 75, 20);
    
    coarseTuneValueLabel.setBounds(coarseTuneSlider.getX(), coarseTuneSlider.getBottom() - 45, coarseTuneSlider.getWidth(), 20);
    fineTuneValueLabel.setBounds(fineTuneSlider.getX(), fineTuneSlider.getBottom() - 45, fineTuneSlider.getWidth(), 20);
    
    //Filter controls
    filterTypeBox.setBounds(370, 265, 100, 25);

    filterCutoffSlider.setBounds(350, 200, sliderSize, sliderSize);
    filterCutoffLabel.setBounds(350, 300, 100, 20);

    filterResonanceSlider.setBounds(420, 200, sliderSize, sliderSize);
    filterResonanceLabel.setBounds(420, 300, 100, 20);
    
    int filterADSR_Y = 200;

    filterAttackSlider.setBounds(20, filterADSR_Y, sliderSize, sliderSize);
    filterDecaySlider.setBounds(90, filterADSR_Y, sliderSize, sliderSize);
    filterSustainSlider.setBounds(160, filterADSR_Y, sliderSize, sliderSize);
    filterReleaseSlider.setBounds(230, filterADSR_Y, sliderSize, sliderSize);
    
    filterAttackLabel.setBounds(filterAttackSlider.getX(), filterAttackSlider.getBottom(), sliderSize, 20);
    filterDecayLabel.setBounds(filterDecaySlider.getX(), filterDecaySlider.getBottom(), sliderSize, 20);
    filterSustainLabel.setBounds(filterSustainSlider.getX(), filterSustainSlider.getBottom(), sliderSize, 20);
    filterReleaseLabel.setBounds(filterReleaseSlider.getX(), filterReleaseSlider.getBottom(), sliderSize, 20);
}

void _1xOscAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
}
