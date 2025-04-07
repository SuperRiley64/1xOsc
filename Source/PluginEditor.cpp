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
    setSize (400, 300);

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
    
    attackSlider.addListener(this);
    decaySlider.addListener(this);
    sustainSlider.addListener(this);
    releaseSlider.addListener(this);
    
    updateADSR();
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
    decaySlider.setBounds(100, adsrYOffset, sliderSize, sliderSize);
    sustainSlider.setBounds(180, adsrYOffset, sliderSize, sliderSize);
    releaseSlider.setBounds(260, adsrYOffset, sliderSize, sliderSize);
}

void _1xOscAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &attackSlider ||
        slider == &decaySlider ||
        slider == &sustainSlider ||
        slider == &releaseSlider)
    {
        updateADSR();
    }
}

void _1xOscAudioProcessorEditor::updateADSR()
{
    // Update the ADSR parameters based on the slider values
    juce::ADSR::Parameters newParams;
    newParams.attack  = (float)attackSlider.getValue();
    newParams.decay   = (float)decaySlider.getValue();
    newParams.sustain = (float)sustainSlider.getValue();
    newParams.release = (float)releaseSlider.getValue();

    // Update the ADSR for all voices in the synthesizer
    for (int i = 0; i < audioProcessor.synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SineWaveVoice*>(audioProcessor.synth.getVoice(i)))
        {
            voice->setADSR(newParams);
        }
    }
}
