#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>
#include "PresetLibrary.h"

class WoodlineAudioProcessor final : public juce::AudioProcessor
{
public:
    WoodlineAudioProcessor();
    ~WoodlineAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Woodline Acoustic Rhythm"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Woodline Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;
    void loadPreset (int presetIndex);
    void clearKeySwitchOverride() noexcept { keySwitchPattern.store (-1); }
    float getOutputLevel() const noexcept { return outputLevel.load(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct Voice
    {
        std::array<float, 8192> delayLine {};
        int delayLength = 64;
        int writePosition = 0;
        float damping = 0.996f;
        float amplitude = 0.0f;
        float envelope = 0.0f;
        float panLeft = 0.707f;
        float panRight = 0.707f;
        bool active = false;
        uint32_t randomState = 0x1234567u;

        void start (int midiNote, float velocity, double sampleRate, float pickAttack,
                    float body, float pan, float fretNoise) noexcept;
        float tick() noexcept;
        float randomBipolar() noexcept;
    };

    struct RenderParameters
    {
        float strumSpeed = 1.0f;
        float humanize = 25.0f;
        float pickAttack = 55.0f;
        float body = 62.0f;
        float bass = 50.0f;
        float mid = 50.0f;
        float treble = 55.0f;
        float fretNoise = 12.0f;
        float reverb = 22.0f;
        float delay = 8.0f;
        float chorus = 5.0f;
        float width = 72.0f;
        float outputDb = 0.0f;
        float tempo = 100.0f;
        int chordMode = 0;
        int voicing = 0;
        int strumStyle = 0;
        int pattern = 0;
        bool latch = false;
        bool loopEnabled = true;
        bool dawSync = true;
    };

    void readParameters (RenderParameters& p) noexcept;
    void handleMidiMessage (const juce::MidiMessage& message) noexcept;
    void renderRange (juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                      const RenderParameters& p) noexcept;
    void renderOneSample (float& left, float& right, const RenderParameters& p) noexcept;
    void beginStrum (int direction, float velocity, const RenderParameters& p) noexcept;
    void triggerNextStrumNote (const RenderParameters& p) noexcept;
    void startVoice (int midiNote, float velocity, float pan, const RenderParameters& p) noexcept;
    void updateHostTempo (RenderParameters& p) noexcept;
    void applyPresetParameter (const juce::String& parameterID, float value);

    std::array<Voice, 24> voices;
    int nextVoice = 0;
    double currentSampleRate = 44100.0;
    std::atomic<float> outputLevel { 0.0f };

    int currentRootNote = -1;
    int lastHeldNote = -1;
    float currentVelocity = 0.8f;
    bool chordHeld = false;
    bool loopRunning = false;
    int activePattern = 0;
    std::atomic<int> keySwitchPattern { -1 };
    int patternStep = 0;
    int samplesUntilNextStep = 0;
    int samplesPerStep = 6000;
    std::array<int, 6> strumNotes {};
    int strumNoteCount = 0;
    int strumNoteIndex = 0;
    int strumDirection = 0;
    int strumDelayRemaining = 0;
    float strumVelocity = 0.8f;
    uint32_t humanizeState = 0x51f15e5u;

    std::array<float, 65536> delayLeft {};
    std::array<float, 65536> delayRight {};
    int delayWritePosition = 0;
    double chorusPhase = 0.0;
    float bodyStateL = 0.0f;
    float bodyStateR = 0.0f;
    juce::Reverb reverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WoodlineAudioProcessor)
};
