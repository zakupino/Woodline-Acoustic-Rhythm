#pragma once
#include <JuceHeader.h>
#include <vector>

struct WoodlinePreset
{
    juce::String name;
    juce::String category;
    juce::String description;
    float strumSpeed = 1.0f;
    float humanize = 24.0f;
    float pickAttack = 55.0f;
    float body = 62.0f;
    float bass = 52.0f;
    float mid = 50.0f;
    float treble = 58.0f;
    float reverb = 24.0f;
    float delay = 8.0f;
    float chorus = 6.0f;
    float width = 72.0f;
    int strumStyle = 0;
    int chordMode = 0;
    int voicing = 0;
    int loopPattern = 0;
};

const std::vector<WoodlinePreset>& getWoodlinePresets();
