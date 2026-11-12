#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    constexpr int patternCount = 8;
    constexpr int stepsPerPattern = 16;
    constexpr int rest = -1;
    const std::array<std::array<int, stepsPerPattern>, patternCount> rhythmPatterns {{
        // Pop eighth-note groove: D - D U - U D U
        {{ 0, rest, 0, 1, rest, 1, 0, 1, 0, rest, 0, 1, rest, 1, 0, 1 }},
        // Ballad: spacious down/up strokes with soft anticipations
        {{ 0, rest, rest, rest, 0, rest, 1, rest, 0, rest, rest, rest, 0, rest, 1, rest }},
        // Straight folk eighths: down/up alternation
        {{ 0, rest, 1, rest, 0, rest, 1, rest, 0, rest, 1, rest, 0, rest, 1, rest }},
        // Country boom-chuck: bass note on 1 and 3, brushed chord on 2 and 4
        {{ 2, rest, rest, rest, 1, rest, rest, rest, 2, rest, rest, rest, 1, rest, rest, rest }},
        // Indie syncopation: off-beat accents and anticipations
        {{ 0, rest, 1, 0, rest, 1, rest, 1, 0, rest, 1, rest, 0, rest, 1, 0 }},
        // Fingerpicked / arpeggiated pulse
        {{ 0, rest, rest, rest, 1, rest, rest, rest, 0, rest, rest, rest, 1, rest, rest, rest }},
        // Rock quarter-note drive
        {{ 0, rest, rest, rest, 0, rest, rest, rest, 0, rest, rest, rest, 0, rest, rest, rest }},
        // Continuous sixteenth-note down/up motion
        {{ 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 }}
    }};

}

int WoodlineAudioProcessor::getPatternStepForDisplay (int pattern, int step) noexcept
{
    return rhythmPatterns[static_cast<size_t> (juce::jlimit (0, patternCount - 1, pattern))]
                         [static_cast<size_t> (juce::jlimit (0, stepsPerPattern - 1, step))];
}

WoodlineAudioProcessor::WoodlineAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "WOODLINE_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout WoodlineAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    const auto floatParam = [&layout] (const char* id, const char* name, float min, float max,
                                       float initial, float interval = 0.1f)
    {
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
                   NormalisableRange<float> (min, max, interval), initial));
    };
    const auto choiceParam = [&layout] (const char* id, const char* name, StringArray choices, int initial)
    {
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, choices, initial));
    };
    const auto boolParam = [&layout] (const char* id, const char* name, bool initial)
    {
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, initial));
    };

    floatParam ("strumSpeed", "Strum Speed", 0.5f, 2.0f, 1.0f, 0.01f);
    floatParam ("humanize", "Humanize", 0.0f, 100.0f, 28.0f, 0.1f);
    floatParam ("tempo", "Tempo", 40.0f, 220.0f, 100.0f, 0.1f);
    floatParam ("pickAttack", "Pick Attack", 0.0f, 100.0f, 56.0f, 0.1f);
    floatParam ("body", "Body Resonance", 0.0f, 100.0f, 62.0f, 0.1f);
    floatParam ("fretNoise", "Fret Noise", 0.0f, 100.0f, 12.0f, 0.1f);
    floatParam ("bass", "Bass", 0.0f, 100.0f, 50.0f, 0.1f);
    floatParam ("mid", "Mid", 0.0f, 100.0f, 50.0f, 0.1f);
    floatParam ("treble", "Treble", 0.0f, 100.0f, 58.0f, 0.1f);
    floatParam ("reverb", "Reverb", 0.0f, 100.0f, 22.0f, 0.1f);
    floatParam ("delay", "Delay", 0.0f, 100.0f, 8.0f, 0.1f);
    floatParam ("chorus", "Chorus", 0.0f, 100.0f, 5.0f, 0.1f);
    floatParam ("width", "Stereo Width", 0.0f, 100.0f, 72.0f, 0.1f);
    floatParam ("output", "Output", -24.0f, 6.0f, 0.0f, 0.1f);

    choiceParam ("strumStyle", "Strum Style", { "Auto", "Down", "Up", "Alternate", "Fingerpick" }, 0);
    choiceParam ("chordMode", "Chord Mode", { "Major", "Minor", "7th", "Maj7", "Sus2", "Power" }, 0);
    choiceParam ("voicing", "Voicing", { "Open", "Barre", "Power", "Solo" }, 0);
    choiceParam ("keySelect", "Key", { "Follow MIDI", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0);
    choiceParam ("loopPattern", "Rhythm Pattern", { "Acoustic Pop 01", "Folk 02", "Indie 03", "Ballad 04", "Country 05", "Sixteenth 06", "Soft 07", "Driving 08" }, 0);
    choiceParam ("theme", "Interface Theme", { "Studio Dark", "Soft Light", "Ocean Blue", "Forest Green", "Aurora Violet", "Ember Gradient" }, 0);

    boolParam ("latch", "Latch", false);
    boolParam ("loopEnabled", "Rhythm Loop", true);
    boolParam ("dawSync", "DAW Sync", true);
    return layout;
}

void WoodlineAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = juce::jmax (22050.0, sampleRate);
    for (auto& voice : voices)
        voice.active = false;
    delayLeft.fill (0.0f);
    delayRight.fill (0.0f);
    delayWritePosition = 0;
    chorusPhase = 0.0;
    bodyStateL = bodyStateR = 0.0f;
    reverb.setSampleRate (currentSampleRate);
    currentRootNote = lastHeldNote = -1;
    chordHeld = loopRunning = false;
    samplesUntilNextStep = 0;
    patternStep = 0;
}

void WoodlineAudioProcessor::releaseResources()
{
    for (auto& voice : voices)
        voice.active = false;
}

bool WoodlineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

float WoodlineAudioProcessor::Voice::randomBipolar() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    const auto value = static_cast<float> ((randomState >> 8) & 0x00ffffffu) / 8388607.5f;
    return value - 1.0f;
}

void WoodlineAudioProcessor::Voice::start (int midiNote, float velocity, double sampleRate,
                                            float pickAttack, float body, float pan,
                                            float fretNoise) noexcept
{
    const auto frequency = juce::MidiMessage::getMidiNoteInHertz (midiNote);
    delayLength = juce::jlimit (2, static_cast<int> (delayLine.size()) - 2,
                                static_cast<int> (std::round (sampleRate / frequency)));
    writePosition = 0;
    damping = 0.9935f + (body / 100.0f) * 0.0057f;
    amplitude = velocity * (0.11f + (pickAttack / 100.0f) * 0.22f);
    envelope = 1.0f;
    const auto noiseAmount = 0.35f + (pickAttack / 100.0f) * 0.5f;
    for (int i = 0; i < delayLength; ++i)
    {
        const auto fundamentalShape = std::sin (juce::MathConstants<float>::twoPi * static_cast<float> (i)
                                                / static_cast<float> (delayLength));
        const auto noise = randomBipolar() * noiseAmount;
        delayLine[static_cast<size_t> (i)] = noise + fundamentalShape * 0.08f
                                                + randomBipolar() * (fretNoise / 100.0f) * 0.035f;
    }
    const auto safePan = juce::jlimit (0.0f, 1.0f, pan);
    panLeft = std::cos (safePan * juce::MathConstants<float>::halfPi);
    panRight = std::sin (safePan * juce::MathConstants<float>::halfPi);
    active = true;
}

float WoodlineAudioProcessor::Voice::tick() noexcept
{
    if (! active)
        return 0.0f;

    const auto current = delayLine[static_cast<size_t> (writePosition)];
    const auto nextPosition = (writePosition + 1) % delayLength;
    const auto next = delayLine[static_cast<size_t> (nextPosition)];
    delayLine[static_cast<size_t> (writePosition)] = (current + next) * 0.5f * damping;
    writePosition = nextPosition;
    envelope *= 0.999975f;
    if (envelope < 0.0007f)
    {
        active = false;
        return 0.0f;
    }
    return current * amplitude * envelope;
}

void WoodlineAudioProcessor::readParameters (RenderParameters& p) noexcept
{
    const auto get = [this] (const char* id, float fallback)
    {
        if (auto* value = parameters.getRawParameterValue (id))
            return value->load();
        return fallback;
    };

    p.strumSpeed = get ("strumSpeed", 1.0f);
    p.humanize = get ("humanize", 28.0f);
    p.tempo = get ("tempo", 100.0f);
    p.pickAttack = get ("pickAttack", 56.0f);
    p.body = get ("body", 62.0f);
    p.fretNoise = get ("fretNoise", 12.0f);
    p.bass = get ("bass", 50.0f);
    p.mid = get ("mid", 50.0f);
    p.treble = get ("treble", 58.0f);
    p.reverb = get ("reverb", 22.0f);
    p.delay = get ("delay", 8.0f);
    p.chorus = get ("chorus", 5.0f);
    p.width = get ("width", 72.0f);
    p.outputDb = get ("output", 0.0f);
    p.chordMode = static_cast<int> (get ("chordMode", 0.0f));
    p.voicing = static_cast<int> (get ("voicing", 0.0f));
    p.strumStyle = static_cast<int> (get ("strumStyle", 0.0f));
    p.pattern = static_cast<int> (get ("loopPattern", 0.0f));
    p.keySelect = static_cast<int> (get ("keySelect", 0.0f));
    p.latch = get ("latch", 0.0f) >= 0.5f;
    p.loopEnabled = get ("loopEnabled", 1.0f) >= 0.5f;
    p.dawSync = get ("dawSync", 1.0f) >= 0.5f;
}

void WoodlineAudioProcessor::updateHostTempo (RenderParameters& p) noexcept
{
    if (! p.dawSync)
        return;
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            if (auto hostBpm = position->getBpm())
                p.tempo = static_cast<float> (*hostBpm);
        }
    }
    p.tempo = juce::jlimit (40.0f, 240.0f, p.tempo);
}

void WoodlineAudioProcessor::handleMidiMessage (const juce::MidiMessage& message) noexcept
{
    if (message.isNoteOn())
    {
        const auto note = message.getNoteNumber();
        if (note >= 24 && note <= 35)
        {
            keySwitchPattern.store ((note - 24) % patternCount);
            activePattern = (note - 24) % patternCount;
            patternStep = 0;
            samplesUntilNextStep = 0;
            return;
        }

        const auto rhythmWasAlreadyRunning = loopRunning && currentRootNote >= 0;
        currentRootNote = note;
        lastHeldNote = note;
        currentVelocity = message.getFloatVelocity();
        chordHeld = true;
        if (! rhythmWasAlreadyRunning)
        {
            patternStep = 0;
            samplesUntilNextStep = 0;
        }
        if (auto* loopParam = parameters.getRawParameterValue ("loopEnabled"))
            loopRunning = loopParam->load() >= 0.5f;
        if (! loopRunning)
        {
            RenderParameters p;
            readParameters (p);
            beginStrum (0, currentVelocity, p);
        }
    }
    else if (message.isNoteOff() || message.isAllNotesOff() || message.isAllSoundOff())
    {
        if (message.isAllNotesOff() || message.isAllSoundOff() || message.getNoteNumber() == lastHeldNote)
        {
            chordHeld = false;
            const auto latchOn = parameters.getRawParameterValue ("latch")->load() >= 0.5f;
            if (! latchOn)
                loopRunning = false;
        }
    }
}

void WoodlineAudioProcessor::beginStrum (int direction, float velocity, const RenderParameters& p) noexcept
{
    static constexpr int major[] = { 0, 7, 12, 16, 19, 24 };
    static constexpr int minor[] = { 0, 7, 12, 15, 19, 24 };
    static constexpr int seventh[] = { 0, 7, 10, 16, 19, 24 };
    static constexpr int majorSeventh[] = { 0, 7, 11, 16, 19, 24 };
    static constexpr int susTwo[] = { 0, 7, 12, 14, 19, 24 };
    static constexpr int power[] = { 0, 7, 12, 19, 24, 31 };

    const int* intervals = major;
    switch (p.chordMode)
    {
        case 1: intervals = minor; break;
        case 2: intervals = seventh; break;
        case 3: intervals = majorSeventh; break;
        case 4: intervals = susTwo; break;
        case 5: intervals = power; break;
        default: break;
    }

    int root = currentRootNote;
    while (root > 52) root -= 12;
    while (root < 36) root += 12;
    if (p.voicing == 1)
        root = juce::jmax (36, root - 12);

    // Follow the incoming MIDI root by default. A manually selected key
    // changes the chord root while keeping the incoming octave.
    if (p.keySelect > 0)
        root = (root / 12) * 12 + (p.keySelect - 1);

    const auto bassOnly = (direction == 2);
    strumNoteCount = (p.voicing == 3 ? 1 : p.voicing == 2 ? 5 : 6);
    if (bassOnly || p.voicing == 3)
    {
        strumNoteCount = 1;
        strumNotes[0] = juce::jlimit (28, 88, root);
    }
    else
    {
        for (int i = 0; i < strumNoteCount; ++i)
            strumNotes[static_cast<size_t> (i)] = juce::jlimit (28, 88, root + intervals[i]);
    }

    strumDirection = bassOnly ? 0 : direction;
    if (! bassOnly && p.voicing != 3)
    {
        if (p.strumStyle == 1) strumDirection = 0;
        if (p.strumStyle == 2) strumDirection = 1;
        if (p.strumStyle == 3) strumDirection = (patternStep % 2 == 0 ? 0 : 1);
        if (p.strumStyle == 4) strumDirection = 0;
    }
    strumNoteIndex = 0;
    if (strumDirection == 1)
        strumNoteIndex = strumNoteCount - 1;
    strumDelayRemaining = 0;
    strumVelocity = velocity;
    triggerNextStrumNote (p);
}

void WoodlineAudioProcessor::triggerNextStrumNote (const RenderParameters& p) noexcept
{
    if (strumNoteIndex < 0 || strumNoteIndex >= strumNoteCount)
        return;

    const auto note = strumNotes[static_cast<size_t> (strumNoteIndex)];
    const auto position = static_cast<float> (strumNoteIndex) / static_cast<float> (juce::jmax (1, strumNoteCount - 1));
    const auto pan = 0.18f + position * 0.64f;
    const auto variation = (humanizeState = humanizeState * 1664525u + 1013904223u,
                            static_cast<float> ((humanizeState >> 8) & 0xffffu) / 65535.0f);
    auto velocityScale = 1.0f - (p.humanize / 100.0f) * 0.22f + variation * (p.humanize / 100.0f) * 0.22f;
    if (p.strumStyle == 4)
        velocityScale *= (strumNoteIndex % 3 == 0 ? 0.94f : 0.68f);
    startVoice (note, strumVelocity * velocityScale, pan, p);

    if (strumDirection == 1)
        --strumNoteIndex;
    else
        ++strumNoteIndex;
    // This control is only the roll time from string to string.
    // At the slow end each string is clearly articulated; at the fast end
    // they form a tight brush. It never changes the DAW-synced beat grid.
    const auto speedPosition = (juce::jlimit (0.5f, 2.0f, p.strumSpeed) - 0.5f) / 1.5f;
    const auto baseDelayMs = (p.strumStyle == 4)
        ? juce::jmap (speedPosition, 58.0f, 16.0f)
        : juce::jmap (speedPosition, 42.0f, 2.5f);
    const auto humanDelay = 1.0f + (variation - 0.5f) * (p.humanize / 100.0f) * 3.0f;
    strumDelayRemaining = juce::jmax (1, static_cast<int> (currentSampleRate * baseDelayMs * humanDelay / 1000.0f));
}

void WoodlineAudioProcessor::startVoice (int midiNote, float velocity, float pan,
                                         const RenderParameters& p) noexcept
{
    auto& voice = voices[static_cast<size_t> (nextVoice++ % static_cast<int> (voices.size()))];
    voice.start (midiNote, juce::jlimit (0.0f, 1.0f, velocity), currentSampleRate,
                 p.pickAttack, p.body, pan, p.fretNoise);
}

void WoodlineAudioProcessor::renderOneSample (float& left, float& right, const RenderParameters& p) noexcept
{
    if (loopRunning && (chordHeld || p.latch) && currentRootNote >= 0)
    {
        if (samplesUntilNextStep <= 0)
        {
            const auto pattern = juce::jlimit (0, patternCount - 1, activePattern);
            const auto direction = rhythmPatterns[static_cast<size_t> (pattern)][static_cast<size_t> (patternStep)];
            if (direction != rest)
                beginStrum (direction, currentVelocity, p);
            patternStep = (patternStep + 1) % stepsPerPattern;
            const auto safeBpm = juce::jlimit (40.0f, 240.0f, p.tempo);
            // Pattern timing follows the host BPM only. Strum Speed is
            // applied separately to the inter-string delay above.
            samplesPerStep = juce::jmax (1, static_cast<int> (currentSampleRate * 60.0 / safeBpm / 4.0));
            samplesUntilNextStep = samplesPerStep;
        }
        --samplesUntilNextStep;
    }

    if (strumNoteIndex >= 0 && strumNoteIndex < strumNoteCount)
    {
        if (strumDelayRemaining <= 0)
            triggerNextStrumNote (p);
        else
            --strumDelayRemaining;
    }

    float dryL = 0.0f;
    float dryR = 0.0f;
    for (auto& voice : voices)
    {
        const auto sample = voice.tick();
        dryL += sample * voice.panLeft;
        dryR += sample * voice.panRight;
    }

    const auto bassGain = juce::Decibels::decibelsToGain ((p.bass - 50.0f) * 0.14f);
    const auto trebleGain = juce::Decibels::decibelsToGain ((p.treble - 50.0f) * 0.12f);
    const auto midGain = juce::Decibels::decibelsToGain ((p.mid - 50.0f) * 0.10f);
    dryL *= bassGain * midGain;
    dryR *= bassGain * midGain;
    const auto bodyAmount = p.body / 100.0f;
    bodyStateL += 0.018f * (dryL - bodyStateL);
    bodyStateR += 0.018f * (dryR - bodyStateR);
    dryL = dryL * (0.75f + trebleGain * 0.12f) + bodyStateL * bodyAmount * 0.28f;
    dryR = dryR * (0.75f + trebleGain * 0.12f) + bodyStateR * bodyAmount * 0.28f;

    const auto delayAmount = p.delay / 100.0f;
    const auto chorusAmount = p.chorus / 100.0f;
    const auto chorusOffset = std::sin (static_cast<float> (chorusPhase)) * chorusAmount * 0.012f;
    chorusPhase += juce::MathConstants<double>::twoPi * 0.23 / currentSampleRate;
    if (chorusPhase >= juce::MathConstants<double>::twoPi)
        chorusPhase -= juce::MathConstants<double>::twoPi;
    const auto delaySecondsL = juce::jlimit (0.04f, 0.42f, 0.22f + chorusOffset);
    const auto delaySecondsR = juce::jlimit (0.04f, 0.42f, 0.29f - chorusOffset);
    const auto delaySamplesL = juce::jlimit (1, static_cast<int> (delayLeft.size()) - 1,
                                             static_cast<int> (currentSampleRate * delaySecondsL));
    const auto delaySamplesR = juce::jlimit (1, static_cast<int> (delayRight.size()) - 1,
                                             static_cast<int> (currentSampleRate * delaySecondsR));
    const auto readL = (delayWritePosition - delaySamplesL + static_cast<int> (delayLeft.size())) % static_cast<int> (delayLeft.size());
    const auto readR = (delayWritePosition - delaySamplesR + static_cast<int> (delayRight.size())) % static_cast<int> (delayRight.size());
    const auto delayedL = delayLeft[static_cast<size_t> (readL)];
    const auto delayedR = delayRight[static_cast<size_t> (readR)];
    delayLeft[static_cast<size_t> (delayWritePosition)] = dryL + delayedL * (0.12f + delayAmount * 0.30f);
    delayRight[static_cast<size_t> (delayWritePosition)] = dryR + delayedR * (0.12f + delayAmount * 0.30f);
    delayWritePosition = (delayWritePosition + 1) % static_cast<int> (delayLeft.size());
    dryL += delayedL * delayAmount * 0.45f;
    dryR += delayedR * delayAmount * 0.45f;

    const auto mid = (dryL + dryR) * 0.5f;
    const auto side = (dryL - dryR) * 0.5f * (p.width / 50.0f);
    const auto outputGain = juce::Decibels::decibelsToGain (p.outputDb);
    left = (mid + side) * outputGain;
    right = (mid - side) * outputGain;
}

void WoodlineAudioProcessor::renderRange (juce::AudioBuffer<float>& buffer, int startSample,
                                          int numSamples, const RenderParameters& p) noexcept
{
    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    for (int i = startSample; i < startSample + numSamples; ++i)
    {
        float l = 0.0f, r = 0.0f;
        renderOneSample (l, r, p);
        left[i] = l;
        if (right != nullptr)
            right[i] = r;
    }
}

void WoodlineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    RenderParameters p;
    readParameters (p);
    updateHostTempo (p);
    displayTempo.store (p.tempo);
    if (! p.loopEnabled)
        loopRunning = false;
    const auto keySwitch = keySwitchPattern.load();
    activePattern = keySwitch >= 0 ? juce::jlimit (0, patternCount - 1, keySwitch)
                                   : juce::jlimit (0, patternCount - 1, p.pattern);

    auto reverbParams = reverb.getParameters();
    reverbParams.roomSize = 0.18f + (p.reverb / 100.0f) * 0.76f;
    reverbParams.damping = 0.35f;
    reverbParams.wetLevel = (p.reverb / 100.0f) * 0.34f;
    reverbParams.dryLevel = 1.0f - (p.reverb / 100.0f) * 0.18f;
    reverbParams.width = p.width / 100.0f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters (reverbParams);

    int lastSample = 0;
    for (const auto metadata : midiMessages)
    {
        const auto eventSample = juce::jlimit (lastSample, buffer.getNumSamples(), metadata.samplePosition);
        if (eventSample > lastSample)
            renderRange (buffer, lastSample, eventSample - lastSample, p);
        handleMidiMessage (metadata.getMessage());
        lastSample = eventSample;
    }
    if (lastSample < buffer.getNumSamples())
        renderRange (buffer, lastSample, buffer.getNumSamples() - lastSample, p);
    midiMessages.clear();

    if (buffer.getNumChannels() > 1)
        reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
    else
        reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());

    const auto level = buffer.getRMSLevel (0, 0, buffer.getNumSamples());
    outputLevel.store (juce::jlimit (0.0f, 1.0f, level * 3.0f));
}

void WoodlineAudioProcessor::applyPresetParameter (const juce::String& parameterID, float value)
{
    if (auto* parameter = parameters.getParameter (parameterID))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
}

void WoodlineAudioProcessor::loadPreset (int presetIndex)
{
    const auto& presets = getWoodlinePresets();
    if (! juce::isPositiveAndBelow (presetIndex, static_cast<int> (presets.size())))
        return;
    const auto& p = presets[static_cast<size_t> (presetIndex)];
    applyPresetParameter ("strumSpeed", p.strumSpeed);
    applyPresetParameter ("humanize", p.humanize);
    applyPresetParameter ("strumStyle", static_cast<float> (p.strumStyle));
    applyPresetParameter ("pickAttack", p.pickAttack);
    applyPresetParameter ("body", p.body);
    applyPresetParameter ("bass", p.bass);
    applyPresetParameter ("mid", p.mid);
    applyPresetParameter ("treble", p.treble);
    applyPresetParameter ("reverb", p.reverb);
    applyPresetParameter ("delay", p.delay);
    applyPresetParameter ("chorus", p.chorus);
    applyPresetParameter ("width", p.width);
    applyPresetParameter ("chordMode", static_cast<float> (p.chordMode));
    applyPresetParameter ("voicing", static_cast<float> (p.voicing));
    applyPresetParameter ("loopPattern", static_cast<float> (p.loopPattern));
}

juce::File WoodlineAudioProcessor::getUserPresetDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("Woodline Audio")
        .getChildFile ("Woodline Acoustic Rhythm")
        .getChildFile ("Presets");
}

juce::Array<juce::File> WoodlineAudioProcessor::getUserPresetFiles() const
{
    auto directory = getUserPresetDirectory();
    directory.createDirectory();
    auto files = directory.findChildFiles (juce::File::findFiles, false, "*.wlpreset");
    files.sort();
    return files;
}

bool WoodlineAudioProcessor::saveUserPreset (const juce::File& requestedFile,
                                               const juce::String& presetName)
{
    if (presetName.trim().isEmpty())
        return false;

    auto file = requestedFile;
    if (file.getFileExtension().compareIgnoreCase (".wlpreset") != 0)
        file = file.withFileExtension ("wlpreset");

    if (file.getParentDirectory().createDirectory().failed())
        return false;

    const auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> stateXml (state.createXml());
    if (stateXml == nullptr)
        return false;

    juce::XmlElement preset ("WOODLINE_USER_PRESET");
    preset.setAttribute ("formatVersion", 1);
    preset.setAttribute ("name", presetName.trim());
    preset.addChildElement (stateXml.release());

    return file.replaceWithText (preset.toString());
}

bool WoodlineAudioProcessor::loadUserPreset (const juce::File& file)
{
    if (! file.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("WOODLINE_USER_PRESET"))
        return false;

    auto* stateElement = xml->getFirstChildElement();
    if (stateElement == nullptr)
        return false;

    auto state = juce::ValueTree::fromXml (*stateElement);
    const auto expectedType = parameters.copyState().getType();
    if (! state.isValid() || state.getType() != expectedType)
        return false;

    parameters.replaceState (state);

    // Imported files are copied into the Woodline User Presets folder so
    // they remain available in the browser on the next plug-in launch.
    const auto userDirectory = getUserPresetDirectory();
    if (file.getFullPathName() != userDirectory.getChildFile (file.getFileName()).getFullPathName())
    {
        userDirectory.createDirectory();
        auto destination = userDirectory.getChildFile (file.getFileName());
        if (destination.existsAsFile())
            destination = userDirectory.getNonexistentChildFile (file.getFileNameWithoutExtension(),
                                                                  ".wlpreset", false);
        file.copyFileTo (destination);
    }
    return true;
}

void WoodlineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    const auto state = parameters.copyState();
    if (const auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void WoodlineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml (*xml);
        if (state.isValid())
            parameters.replaceState (state);
    }
}

juce::AudioProcessorEditor* WoodlineAudioProcessor::createEditor()
{
    return new WoodlineAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WoodlineAudioProcessor();
}
