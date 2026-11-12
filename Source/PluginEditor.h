#pragma once

#include <JuceHeader.h>
#include <array>
#include <functional>
#include <memory>
#include "PluginProcessor.h"
#include "Theme.h"

class PresetBrowserComponent final : public juce::Component
{
public:
    PresetBrowserComponent();
    void setPalette (const WoodlinePalette& newPalette);
    void setSelectedPreset (int index);
    void setOnPresetSelected (std::function<void (int)> callback);
    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;

private:
    std::vector<int> getVisiblePresets() const;
    juce::Rectangle<int> getGridBounds() const;
    juce::Rectangle<int> getCardBounds (int visibleCard) const;
    void drawGuitarArtwork (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour accent) const;

    WoodlinePalette palette = getWoodlinePalette (0);
    std::function<void (int)> onPresetSelected;
    juce::String selectedCategory = "All Presets";
    int selectedPreset = 0;
    int pageIndex = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBrowserComponent)
};

class WoodlineLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    WoodlineLookAndFeel();
    void setPalette (const WoodlinePalette& newPalette);
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
private:
    WoodlinePalette palette = getWoodlinePalette (0);
};

class WoodlineAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit WoodlineAudioProcessorEditor (WoodlineAudioProcessor&);
    ~WoodlineAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct Knob
    {
        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<SliderAttachment> attachment;
    };

    void setupKnob (int index, const juce::String& parameterID, const juce::String& labelText);
    void setupCombo (juce::ComboBox& box, const juce::StringArray& items);
    void applyPalette (int themeIndex);
    void timerCallback() override;
    void drawPanel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title,
                    const juce::String& subtitle = {}) const;
    void placeKnob (int index, juce::Rectangle<int> area);

    WoodlineAudioProcessor& processor;
    WoodlineLookAndFeel woodlineLookAndFeel;
    PresetBrowserComponent presetBrowser;
    WoodlinePalette palette = getWoodlinePalette (0);
    std::array<Knob, 14> knobs;
    std::array<juce::Rectangle<int>, 5> panelBounds;

    juce::ComboBox strumStyleBox, chordModeBox, voicingBox, loopPatternBox, themeBox, keySelectBox;
    juce::TextButton latchButton { "LATCH" }, loopButton { "RHYTHM LOOP" }, syncButton { "DAW SYNC" };
    juce::Label strumStyleLabel, chordModeLabel, voicingLabel, loopPatternLabel, themeLabel, keyLabel;
    std::unique_ptr<ComboBoxAttachment> strumStyleAttachment, chordModeAttachment,
        voicingAttachment, loopPatternAttachment, themeAttachment, keySelectAttachment;
    std::unique_ptr<ButtonAttachment> latchAttachment, loopAttachment, syncAttachment;
    float displayedOutputLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WoodlineAudioProcessorEditor)
};
