/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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

void _1xOscAudioProcessorEditor::resized()
{
    waveformComboBox.setBounds(150, 100, 100, 30);
    waveformLabel.setBounds(150, 70, 100, 20);
}
