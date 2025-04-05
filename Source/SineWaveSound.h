/*
  ==============================================================================

    SineWaveSound.h
    Created: 3 Apr 2025 4:16:40pm
    Author:  Riley Knybel

  ==============================================================================
*/

#pragma once

class SineWaveSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};
