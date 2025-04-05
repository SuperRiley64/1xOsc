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
    
    // Setup waveform selector knob
        waveformSelector.setSliderStyle(juce::Slider::Rotary);
        waveformSelector.setRange(0, 4, 1); // 5 positions: 0-Sine, 1-Square, 2-Triangle, 3-Saw, 4-Noise
        waveformSelector.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        addAndMakeVisible(waveformSelector);

        // Label for the waveform selector
        waveformLabel.setText("Waveform", juce::dontSendNotification);
        waveformLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(waveformLabel);
    
    // Attach the slider to the parameter
    waveformAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "waveform", waveformSelector);
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
    waveformSelector.setBounds(150, 100, 100, 100);
    waveformLabel.setBounds(150, 200, 100, 20);
}
