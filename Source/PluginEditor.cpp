#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

namespace
{
    const juce::StringArray browserCategories { "All Presets", "Strumming", "Fingerstyle",
        "Pop & Ballad", "Rock", "Country", "Cinematic", "User Presets", "Favorites" };

    juce::Colour withAlpha (juce::Colour colour, float alpha)
    {
        return colour.withAlpha (juce::jlimit (0.0f, 1.0f, alpha));
    }
}

PresetBrowserComponent::PresetBrowserComponent()
{
}

void PresetBrowserComponent::setPalette (const WoodlinePalette& newPalette)
{
    palette = newPalette;
    repaint();
}

void PresetBrowserComponent::setSelectedPreset (int index)
{
    selectedPreset = juce::jlimit (0, static_cast<int> (getWoodlinePresets().size()) - 1, index);
    repaint();
}

void PresetBrowserComponent::setOnPresetSelected (std::function<void (int)> callback)
{
    onPresetSelected = std::move (callback);
}

void PresetBrowserComponent::resized()
{
}

std::vector<int> PresetBrowserComponent::getVisiblePresets() const
{
    std::vector<int> visible;
    const auto& all = getWoodlinePresets();
    for (int i = 0; i < static_cast<int> (all.size()); ++i)
    {
        const auto& preset = all[static_cast<size_t> (i)];
        const bool categoryMatch = selectedCategory == "All Presets" || preset.category == selectedCategory
            || (selectedCategory == "Favorites" && (i == 0 || i == 12 || i == 24 || i == 36 || i == 48));
        if (categoryMatch && selectedCategory != "User Presets")
            visible.push_back (i);
    }
    return visible;
}

juce::Rectangle<int> PresetBrowserComponent::getGridBounds() const
{
    auto content = getLocalBounds().reduced (10);
    content.removeFromTop (48);
    content.removeFromLeft (154);
    content.removeFromRight (208);
    content.removeFromBottom (27);
    return content.reduced (4, 3);
}

juce::Rectangle<int> PresetBrowserComponent::getCardBounds (int visibleCard) const
{
    auto grid = getGridBounds();
    const int gap = 10;
    const int cardWidth = (grid.getWidth() - gap * 2) / 3;
    const int cardHeight = (grid.getHeight() - gap) / 2;
    const int col = visibleCard % 3;
    const int row = visibleCard / 3;
    return { grid.getX() + col * (cardWidth + gap), grid.getY() + row * (cardHeight + gap), cardWidth, cardHeight };
}

void PresetBrowserComponent::drawGuitarArtwork (juce::Graphics& g, juce::Rectangle<float> area,
                                                juce::Colour accent) const
{
    auto body = area.withSizeKeepingCentre (area.getWidth() * 0.22f, area.getHeight() * 0.64f);
    juce::Path guitar;
    guitar.startNewSubPath (body.getCentreX(), body.getY());
    guitar.cubicTo (body.getRight() * 0.98f, body.getY() + body.getHeight() * 0.15f,
                    body.getRight() * 1.10f, body.getY() + body.getHeight() * 0.44f,
                    body.getCentreX(), body.getBottom());
    guitar.cubicTo (body.getX() - body.getWidth() * 0.10f, body.getY() + body.getHeight() * 0.44f,
                    body.getX() + body.getWidth() * 0.02f, body.getY() + body.getHeight() * 0.15f,
                    body.getCentreX(), body.getY());
    guitar.closeSubPath();
    g.setColour (withAlpha (accent, 0.23f));
    g.fillPath (guitar);
    g.setColour (withAlpha (juce::Colours::white, 0.68f));
    g.strokePath (guitar, juce::PathStrokeType (1.5f));
    const auto neckX = body.getCentreX() - body.getWidth() * 0.10f;
    g.drawLine (neckX, body.getY() - area.getHeight() * 0.34f, neckX, body.getY() + body.getHeight() * 0.36f, 3.0f);
    g.setColour (withAlpha (juce::Colours::white, 0.72f));
    g.drawEllipse (body.getCentreX() - body.getWidth() * 0.17f, body.getCentreY() - body.getWidth() * 0.17f,
                   body.getWidth() * 0.34f, body.getWidth() * 0.34f, 1.2f);
}

void PresetBrowserComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (palette.panelRaised, bounds.getTopLeft(), palette.panel, bounds.getBottomRight(), false));
    g.fillRoundedRectangle (bounds, 12.0f);
    g.setColour (palette.border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 12.0f, 1.0f);

    // A compact library header replaces the unused search bar.
    g.setColour (palette.text);
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("GUITAR PRESET LIBRARY", 20, 10, 220, 18, juce::Justification::centredLeft);
    g.setColour (palette.muted);
    g.setFont (juce::Font (10.0f));
    g.drawText ("Browse every playing style • rhythm follows your DAW tempo", 20, 27, 340, 14,
                juce::Justification::centredLeft);

    const auto visible = getVisiblePresets();
    const auto pageCount = juce::jmax (1, (static_cast<int> (visible.size()) + 5) / 6);
    pageIndex = juce::jlimit (0, pageCount - 1, pageIndex);

    auto previous = juce::Rectangle<int> (getWidth() - 150, 12, 24, 24);
    auto pageLabel = juce::Rectangle<int> (getWidth() - 121, 13, 82, 22);
    auto next = juce::Rectangle<int> (getWidth() - 35, 12, 24, 24);
    g.setColour (palette.field);
    g.fillRoundedRectangle (previous.toFloat(), 6.0f);
    g.fillRoundedRectangle (next.toFloat(), 6.0f);
    g.setColour (palette.border);
    g.drawRoundedRectangle (previous.toFloat(), 6.0f, 1.0f);
    g.drawRoundedRectangle (next.toFloat(), 6.0f, 1.0f);
    g.setColour (palette.text);
    g.setFont (juce::Font (16.0f, juce::Font::bold));
    g.drawText ("‹", previous, juce::Justification::centred);
    g.drawText ("›", next, juce::Justification::centred);
    g.setFont (juce::Font (10.5f, juce::Font::bold));
    g.drawText ("PAGE " + juce::String (pageIndex + 1) + " / " + juce::String (pageCount),
                pageLabel, juce::Justification::centred);

    auto content = getLocalBounds().reduced (10);
    content.removeFromTop (48);
    auto categories = content.removeFromLeft (145);
    content.removeFromLeft (8);
    auto details = content.removeFromRight (198);
    content.removeFromLeft (8);
    auto grid = content.reduced (2, 2);
    grid.removeFromBottom (27);

    g.setColour (palette.field);
    g.fillRoundedRectangle (categories.toFloat(), 8.0f);
    g.setColour (palette.muted);
    g.setFont (juce::Font (10.5f, juce::Font::bold));
    g.drawText ("GUITAR STYLES", categories.reduced (12).removeFromTop (25), juce::Justification::centredLeft);

    int categoryY = categories.getY() + 34;
    for (int i = 0; i < browserCategories.size(); ++i)
    {
        auto row = juce::Rectangle<int> (categories.getX() + 5, categoryY + i * 29, categories.getWidth() - 10, 26);
        const bool active = selectedCategory == browserCategories[i];
        if (active)
        {
            g.setColour (withAlpha (palette.accent, 0.23f));
            g.fillRoundedRectangle (row.toFloat(), 6.0f);
            g.setColour (palette.accent);
            g.fillRoundedRectangle (juce::Rectangle<float> (row.getX(), row.getY() + 4, 3.0f, row.getHeight() - 8), 2.0f);
        }
        g.setColour (active ? palette.text : palette.muted);
        g.setFont (juce::Font (11.5f, active ? juce::Font::bold : juce::Font::plain));
        g.drawText (browserCategories[i], row.withTrimmedLeft (12), juce::Justification::centredLeft, true);
    }

    if (selectedCategory == "User Presets")
    {
        g.setColour (palette.muted);
        g.setFont (juce::Font (14.0f));
        g.drawFittedText ("USER PRESETS\nYour saved presets will appear here in a future update.\nFactory presets and current DAW state remain available.",
                          grid, juce::Justification::centred, 3);
    }
    else if (visible.empty())
    {
        g.setColour (palette.muted);
        g.setFont (juce::Font (14.0f));
        g.drawFittedText ("No presets in this view.", grid, juce::Justification::centred, 1);
    }
    else
    {
        const auto firstPreset = pageIndex * 6;
        for (int cardIndex = 0; cardIndex < 6 && firstPreset + cardIndex < static_cast<int> (visible.size()); ++cardIndex)
        {
            const auto presetIndex = visible[static_cast<size_t> (firstPreset + cardIndex)];
            const auto& preset = getWoodlinePresets()[static_cast<size_t> (presetIndex)];
            auto card = getCardBounds (cardIndex).toFloat();
            const auto selected = presetIndex == selectedPreset;
            const auto hue = static_cast<float> ((presetIndex * 37) % 110) / 360.0f;
            const auto artwork = juce::Colour::fromHSV (hue, 0.38f, 0.54f, 1.0f);
            g.setGradientFill (juce::ColourGradient (artwork.withAlpha (0.60f), card.getTopLeft(), palette.card, card.getBottomRight(), false));
            g.fillRoundedRectangle (card, 8.0f);
            g.setColour (selected ? palette.accentBright : palette.border);
            g.drawRoundedRectangle (card, 8.0f, selected ? 2.0f : 1.0f);
            drawGuitarArtwork (g, card.reduced (5.0f, 4.0f).withTrimmedBottom (36.0f), palette.accentBright);
            g.setColour (withAlpha (juce::Colours::black, 0.68f));
            g.fillRoundedRectangle (juce::Rectangle<float> (card.getX() + 1.0f, card.getBottom() - 43.0f,
                                                             card.getWidth() - 2.0f, 42.0f), 7.0f);
            g.setColour (palette.text);
            g.setFont (juce::Font (12.5f, juce::Font::bold));
            g.drawText (preset.name, juce::Rectangle<int> (static_cast<int> (card.getX() + 9), static_cast<int> (card.getBottom() - 38),
                                                           static_cast<int> (card.getWidth() - 16), 18), juce::Justification::centredLeft, true);
            g.setColour (palette.muted);
            g.setFont (juce::Font (10.0f));
            const auto articulation = preset.strumStyle == 4 ? "Fingerpicked" : "Strummed";
            g.drawText (preset.category + "  •  " + articulation,
                        juce::Rectangle<int> (static_cast<int> (card.getX() + 9), static_cast<int> (card.getBottom() - 20),
                                              static_cast<int> (card.getWidth() - 16), 15), juce::Justification::centredLeft, true);
            if (selected)
            {
                g.setColour (palette.accentBright);
                g.fillEllipse (card.getRight() - 22.0f, card.getY() + 9.0f, 11.0f, 11.0f);
            }
        }
    }

    // Details panel includes a useful pattern preview instead of fake BPM/waveform metadata.
    g.setColour (palette.field);
    g.fillRoundedRectangle (details.toFloat(), 8.0f);
    g.setColour (palette.border);
    g.drawRoundedRectangle (details.toFloat(), 8.0f, 1.0f);
    const auto& all = getWoodlinePresets();
    const auto safeSelected = juce::jlimit (0, static_cast<int> (all.size()) - 1, selectedPreset);
    const auto& selected = all[static_cast<size_t> (safeSelected)];
    auto info = details.reduced (12);
    g.setColour (palette.muted);
    g.setFont (juce::Font (10.0f, juce::Font::bold));
    g.drawText ("SELECTED PRESET", info.removeFromTop (18), juce::Justification::centredLeft);
    info.removeFromTop (5);
    g.setColour (palette.text);
    g.setFont (juce::Font (16.0f, juce::Font::bold));
    g.drawFittedText (selected.name, info.removeFromTop (38), juce::Justification::centredLeft, 2);
    g.setColour (palette.muted);
    g.setFont (juce::Font (10.5f));
    g.drawFittedText (selected.description, info.removeFromTop (58), juce::Justification::topLeft, 4);
    info.removeFromTop (3);
    g.setColour (palette.accentBright);
    g.setFont (juce::Font (10.5f, juce::Font::bold));
    g.drawText (selected.category + "  •  " + (selected.strumStyle == 4 ? "FINGERSTYLE" : "STRUM FEEL"),
                info.removeFromTop (18), juce::Justification::centredLeft);

    auto patternArea = info.removeFromTop (51).reduced (2, 4);
    g.setColour (withAlpha (palette.accent, 0.18f));
    g.fillRoundedRectangle (patternArea.toFloat(), 5.0f);
    const auto pattern = selected.loopPattern;
    for (int step = 0; step < 16; ++step)
    {
        const auto event = WoodlineAudioProcessor::getPatternStepForDisplay (pattern, step);
        const auto x = static_cast<float> (patternArea.getX() + 5)
                     + static_cast<float> (step) * static_cast<float> (patternArea.getWidth() - 10) / 16.0f;
        const auto centreY = static_cast<float> (patternArea.getCentreY());
        if (event >= 0)
        {
            g.setColour (event == 2 ? palette.accentBright : palette.text);
            const auto barHeight = event == 2 ? 13.0f : (event == 0 ? 19.0f : 12.0f);
            g.fillRoundedRectangle (x, centreY - barHeight * 0.5f, 3.0f, barHeight, 1.3f);
        }
        else
        {
            g.setColour (withAlpha (palette.muted, 0.45f));
            g.fillEllipse (x, centreY - 1.5f, 3.0f, 3.0f);
        }
    }

    auto loadButton = info.removeFromBottom (32);
    g.setGradientFill (juce::ColourGradient (palette.accent, loadButton.toFloat().getTopLeft(), palette.accentBright,
                                              loadButton.toFloat().getBottomRight(), false));
    g.fillRoundedRectangle (loadButton.toFloat(), 7.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (11.5f, juce::Font::bold));
    g.drawText ("LOAD PRESET", loadButton, juce::Justification::centred);

    // Page navigation is also labelled so all 60 factory presets remain reachable.
    auto pageArea = juce::Rectangle<int> (categories.getRight() + 15, getHeight() - 27,
                                          getWidth() - categories.getRight() - 230, 20);
    g.setColour (palette.muted);
    g.setFont (juce::Font (9.5f));
    g.drawText ("ALL FACTORY PRESETS  •  " + juce::String (visible.size()) + " IN THIS STYLE",
                pageArea, juce::Justification::centredLeft);
}

void PresetBrowserComponent::mouseDown (const juce::MouseEvent& event)
{
    const auto point = event.getPosition();
    const auto visible = getVisiblePresets();
    const auto pageCount = juce::jmax (1, (static_cast<int> (visible.size()) + 5) / 6);

    if (point.y >= 8 && point.y <= 42)
    {
        if (point.x >= getWidth() - 150 && point.x <= getWidth() - 126)
            pageIndex = juce::jmax (0, pageIndex - 1);
        else if (point.x >= getWidth() - 35 && point.x <= getWidth() - 11)
            pageIndex = juce::jmin (pageCount - 1, pageIndex + 1);
        repaint();
        return;
    }

    auto content = getLocalBounds().reduced (10);
    content.removeFromTop (48);
    auto categories = content.removeFromLeft (145);
    if (categories.contains (point))
    {
        const auto row = (point.y - (categories.getY() + 34)) / 29;
        if (juce::isPositiveAndBelow (row, browserCategories.size()))
        {
            selectedCategory = browserCategories[row];
            pageIndex = 0;
            repaint();
        }
        return;
    }

    const auto firstPreset = pageIndex * 6;
    for (int i = 0; i < 6 && firstPreset + i < static_cast<int> (visible.size()); ++i)
    {
        if (getCardBounds (i).contains (point))
        {
            selectedPreset = visible[static_cast<size_t> (firstPreset + i)];
            if (onPresetSelected)
                onPresetSelected (selectedPreset);
            repaint();
            return;
        }
    }
    if (point.x > getWidth() - 216 && point.y > getHeight() - 62)
    {
        if (onPresetSelected)
            onPresetSelected (selectedPreset);
    }
}

WoodlineLookAndFeel::WoodlineLookAndFeel()
{
    setPalette (getWoodlinePalette (0));
}

void WoodlineLookAndFeel::setPalette (const WoodlinePalette& newPalette)
{
    palette = newPalette;
    setColour (juce::Slider::thumbColourId, palette.accentBright);
    setColour (juce::Slider::rotarySliderFillColourId, palette.accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, palette.border);
    setColour (juce::Slider::textBoxTextColourId, palette.text);
    setColour (juce::Slider::textBoxBackgroundColourId, palette.field);
    setColour (juce::Slider::textBoxOutlineColourId, palette.border);
    setColour (juce::ComboBox::backgroundColourId, palette.field);
    setColour (juce::ComboBox::textColourId, palette.text);
    setColour (juce::ComboBox::outlineColourId, palette.border);
    setColour (juce::PopupMenu::backgroundColourId, palette.panelRaised);
    setColour (juce::PopupMenu::textColourId, palette.text);
    setColour (juce::TextButton::buttonColourId, palette.field);
    setColour (juce::TextButton::buttonOnColourId, withAlpha (palette.accent, 0.45f));
    setColour (juce::TextButton::textColourOffId, palette.text);
    setColour (juce::TextButton::textColourOnId, palette.text);
}

void WoodlineLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    juce::ignoreUnused (slider);
    const auto radius = static_cast<float> (juce::jmin (width, height)) * 0.34f;
    const auto centreX = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
    const auto centreY = static_cast<float> (y) + static_cast<float> (height) * 0.46f;
    const auto rx = centreX - radius;
    const auto ry = centreY - radius;
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (withAlpha (juce::Colours::black, 0.32f));
    g.fillEllipse (rx - 3.0f, ry + 3.0f, radius * 2.0f + 6.0f, radius * 2.0f + 6.0f);
    g.setGradientFill (juce::ColourGradient (palette.panelRaised.brighter (0.18f), rx, ry,
                                              palette.field.darker (0.35f), rx + radius * 2.0f, ry + radius * 2.0f, false));
    g.fillEllipse (rx, ry, radius * 2.0f, radius * 2.0f);
    g.setColour (palette.border);
    g.drawEllipse (rx, ry, radius * 2.0f, radius * 2.0f, 1.4f);

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centreX, centreY, radius + 4.0f, radius + 4.0f, 0.0f,
                                 rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (palette.border);
    g.strokePath (backgroundArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius + 4.0f, radius + 4.0f, 0.0f,
                            rotaryStartAngle, angle, true);
    g.setColour (palette.accentBright);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.addRoundedRectangle (-1.6f, -radius + 6.0f, 3.2f, radius * 0.48f, 1.5f);
    g.setColour (palette.text);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (withAlpha (palette.accent, 0.22f));
    g.fillEllipse (centreX - radius * 0.18f, centreY - radius * 0.18f, radius * 0.36f, radius * 0.36f);
}

WoodlineAudioProcessorEditor::WoodlineAudioProcessorEditor (WoodlineAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&woodlineLookAndFeel);
    addAndMakeVisible (presetBrowser);
    presetBrowser.setOnPresetSelected ([this] (int index)
    {
        processor.loadPreset (index);
        presetBrowser.setSelectedPreset (index);
    });

    setupKnob (0, "strumSpeed", "STRUM SPEED");
    setupKnob (1, "humanize", "HUMANIZE");
    setupKnob (2, "tempo", "TEMPO");
    setupKnob (3, "pickAttack", "PICK ATTACK");
    setupKnob (4, "body", "BODY");
    setupKnob (5, "fretNoise", "FRET NOISE");
    setupKnob (6, "bass", "BASS");
    setupKnob (7, "mid", "MID");
    setupKnob (8, "treble", "TREBLE");
    setupKnob (9, "reverb", "REVERB");
    setupKnob (10, "delay", "DELAY");
    setupKnob (11, "chorus", "CHORUS");
    setupKnob (12, "width", "WIDTH");
    setupKnob (13, "output", "OUTPUT");

    setupCombo (strumStyleBox, { "Auto", "Down", "Up", "Alternate", "Fingerpick" });
    setupCombo (chordModeBox, { "Major", "Minor", "7th", "Maj7", "Sus2", "Power" });
    setupCombo (voicingBox, { "Open", "Barre", "Power", "Solo" });
    setupCombo (keySelectBox, { "Follow MIDI", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" });
    setupCombo (loopPatternBox, { "Acoustic Pop 01", "Folk 02", "Indie 03", "Ballad 04",
                                  "Country 05", "Sixteenth 06", "Soft 07", "Driving 08" });
    setupCombo (themeBox, { "Studio Dark", "Soft Light", "Ocean Blue", "Forest Green", "Aurora Violet", "Ember Gradient" });

    strumStyleAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "strumStyle", strumStyleBox);
    chordModeAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "chordMode", chordModeBox);
    voicingAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "voicing", voicingBox);
    loopPatternAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "loopPattern", loopPatternBox);
    themeAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "theme", themeBox);
    keySelectAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "keySelect", keySelectBox);
    latchButton.setClickingTogglesState (true);
    loopButton.setClickingTogglesState (true);
    syncButton.setClickingTogglesState (true);
    latchAttachment = std::make_unique<ButtonAttachment> (processor.parameters, "latch", latchButton);
    loopAttachment = std::make_unique<ButtonAttachment> (processor.parameters, "loopEnabled", loopButton);
    syncAttachment = std::make_unique<ButtonAttachment> (processor.parameters, "dawSync", syncButton);

    const std::array<juce::Component*, 8> controls {{
        &strumStyleBox, &chordModeBox, &voicingBox, &loopPatternBox,
        &themeBox, &keySelectBox, &latchButton, &loopButton, &syncButton
    }};
    for (auto* component : controls)
        addAndMakeVisible (component);

    strumStyleLabel.setText ("STRUM STYLE", juce::dontSendNotification);
    keyLabel.setText ("KEY / MIDI ROOT", juce::dontSendNotification);
    chordModeLabel.setText ("CHORD MODE", juce::dontSendNotification);
    voicingLabel.setText ("VOICING", juce::dontSendNotification);
    loopPatternLabel.setText ("RHYTHM LOOP", juce::dontSendNotification);
    keyLabel.setText ("KEY / MIDI ROOT", juce::dontSendNotification);
    themeLabel.setText ("THEME", juce::dontSendNotification);
    for (auto* label : { &strumStyleLabel, &chordModeLabel, &voicingLabel, &loopPatternLabel, &themeLabel, &keyLabel })
    {
        label->setFont (juce::Font (10.5f, juce::Font::bold));
        addAndMakeVisible (*label);
    }

    themeBox.onChange = [this] { applyPalette (themeBox.getSelectedItemIndex()); };
    loopPatternBox.onChange = [this] { processor.clearKeySwitchOverride(); };
    applyPalette (static_cast<int> (processor.parameters.getRawParameterValue ("theme")->load()));
    presetBrowser.setSelectedPreset (0);

    setResizable (true, true);
    setResizeLimits (1000, 720, 1800, 1120);
    setSize (1260, 840);
    startTimerHz (20);
}

WoodlineAudioProcessorEditor::~WoodlineAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void WoodlineAudioProcessorEditor::setupKnob (int index, const juce::String& parameterID,
                                                const juce::String& labelText)
{
    auto& knob = knobs[static_cast<size_t> (index)];
    knob.label.setText (labelText, juce::dontSendNotification);
    knob.label.setJustificationType (juce::Justification::centred);
    knob.label.setFont (juce::Font (9.5f, juce::Font::bold));
    knob.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 54, 17);
    knob.slider.setPopupDisplayEnabled (true, true, this);
    knob.slider.setVelocityBasedMode (true);
    knob.attachment = std::make_unique<SliderAttachment> (processor.parameters, parameterID, knob.slider);
    addAndMakeVisible (knob.label);
    addAndMakeVisible (knob.slider);
}

void WoodlineAudioProcessorEditor::setupCombo (juce::ComboBox& box, const juce::StringArray& items)
{
    box.addItemList (items, 1);
    box.setJustificationType (juce::Justification::centredLeft);
    box.setScrollWheelEnabled (false);
}

void WoodlineAudioProcessorEditor::applyPalette (int themeIndex)
{
    palette = getWoodlinePalette (themeIndex);
    woodlineLookAndFeel.setPalette (palette);
    presetBrowser.setPalette (palette);
    for (auto& knob : knobs)
    {
        knob.label.setColour (juce::Label::textColourId, palette.muted);
        knob.slider.setColour (juce::Slider::textBoxTextColourId, palette.text);
        knob.slider.setColour (juce::Slider::textBoxBackgroundColourId, palette.field);
        knob.slider.setColour (juce::Slider::textBoxOutlineColourId, palette.border);
    }
    for (auto* combo : { &strumStyleBox, &chordModeBox, &voicingBox, &loopPatternBox, &themeBox, &keySelectBox })
    {
        combo->setColour (juce::ComboBox::backgroundColourId, palette.field);
        combo->setColour (juce::ComboBox::textColourId, palette.text);
        combo->setColour (juce::ComboBox::outlineColourId, palette.border);
        combo->setColour (juce::ComboBox::arrowColourId, palette.accentBright);
    }
    for (auto* button : { &latchButton, &loopButton, &syncButton })
    {
        button->setColour (juce::TextButton::buttonColourId, palette.field);
        button->setColour (juce::TextButton::buttonOnColourId, withAlpha (palette.accent, 0.45f));
        button->setColour (juce::TextButton::textColourOffId, palette.text);
        button->setColour (juce::TextButton::textColourOnId, palette.text);
    }
    for (auto* label : { &strumStyleLabel, &chordModeLabel, &voicingLabel, &loopPatternLabel, &themeLabel, &keyLabel })
        label->setColour (juce::Label::textColourId, palette.muted);
    repaint();
}

void WoodlineAudioProcessorEditor::drawPanel (juce::Graphics& g, juce::Rectangle<int> bounds,
                                                const juce::String& title, const juce::String& subtitle) const
{
    auto area = bounds.toFloat();
    g.setGradientFill (juce::ColourGradient (palette.panelRaised, area.getTopLeft(), palette.panel, area.getBottomRight(), false));
    g.fillRoundedRectangle (area, 10.0f);
    g.setColour (palette.border);
    g.drawRoundedRectangle (area.reduced (0.5f), 10.0f, 1.0f);
    g.setColour (palette.accent);
    g.fillRoundedRectangle (juce::Rectangle<float> (area.getX() + 10.0f, area.getY() + 10.0f, 4.0f, 22.0f), 2.0f);
    g.setColour (palette.text);
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText (title, bounds.getX() + 22, bounds.getY() + 10, bounds.getWidth() - 30, 20, juce::Justification::centredLeft);
    if (subtitle.isNotEmpty())
    {
        g.setColour (palette.muted);
        g.setFont (juce::Font (9.5f));
        g.drawText (subtitle, bounds.getX() + 22, bounds.getY() + 29, bounds.getWidth() - 30, 15, juce::Justification::centredLeft);
    }
    g.setColour (withAlpha (palette.border, 0.8f));
    g.drawLine (static_cast<float> (bounds.getX() + 10), static_cast<float> (bounds.getY() + 43),
                static_cast<float> (bounds.getRight() - 10), static_cast<float> (bounds.getY() + 43), 1.0f);
}

void WoodlineAudioProcessorEditor::placeKnob (int index, juce::Rectangle<int> area)
{
    auto& knob = knobs[static_cast<size_t> (index)];
    knob.label.setBounds (area.removeFromTop (17));
    knob.slider.setBounds (area);
}

void WoodlineAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (palette.background.brighter (0.03f), 0.0f, 0.0f,
                                              palette.background, 0.0f, static_cast<float> (getHeight()), false));
    g.fillAll();

    auto header = juce::Rectangle<int> (0, 0, getWidth(), 56);
    g.setGradientFill (juce::ColourGradient (palette.panelRaised, header.toFloat().getTopLeft(), palette.panel, header.toFloat().getBottomRight(), false));
    g.fillRect (header);
    g.setColour (palette.border);
    g.drawLine (0.0f, 55.5f, static_cast<float> (getWidth()), 55.5f, 1.0f);

    g.setColour (palette.accentBright);
    g.fillEllipse (17.0f, 14.0f, 27.0f, 27.0f);
    g.setColour (palette.background);
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawText ("W", 17, 15, 27, 25, juce::Justification::centred);
    g.setColour (palette.text);
    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.drawText ("WOODLINE", 55, 8, 150, 24, juce::Justification::centredLeft);
    g.setColour (palette.muted);
    g.setFont (juce::Font (10.5f, juce::Font::bold));
    g.drawText ("ACOUSTIC RHYTHM GUITAR  •  VST3", 57, 32, 280, 16, juce::Justification::centredLeft);

    const auto meterX = static_cast<float> (getWidth() - 164);
    g.setColour (palette.muted);
    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.drawText ("OUTPUT", static_cast<int> (meterX) - 34, 19, 48, 16, juce::Justification::centredRight);
    for (int i = 0; i < 18; ++i)
    {
        const auto x = meterX + static_cast<float> (i) * 7.0f;
        const auto active = displayedOutputLevel > static_cast<float> (i) / 22.0f;
        g.setColour (active ? palette.accentBright : palette.border);
        g.fillRoundedRectangle (x, 17.0f, 4.0f, 17.0f, 1.4f);
    }
    g.setColour (palette.muted);
    g.setFont (juce::Font (9.0f));
    g.drawText ("60 FACTORY PRESETS  |  MIDI IN", getWidth() - 330, 38, 310, 12, juce::Justification::centredRight);

    for (int i = 0; i < static_cast<int> (panelBounds.size()); ++i)
    {
        static const juce::String names[] = { "STRUM ENGINE", "VOICING", "RHYTHM LOOP", "TONE SHAPING", "FX RACK" };
        static const juce::String subtitles[] = { "Feel & articulation", "One-finger chord engine", "DAW-synced patterns", "Acoustic body & EQ", "Space & stereo" };
        drawPanel (g, panelBounds[static_cast<size_t> (i)], names[i], subtitles[i]);
    }

    const auto boardY = getHeight() - 91;
    auto board = juce::Rectangle<int> (12, boardY, getWidth() - 24, 78);
    g.setGradientFill (juce::ColourGradient (palette.panelRaised, board.toFloat().getTopLeft(), palette.panel, board.toFloat().getBottomRight(), false));
    g.fillRoundedRectangle (board.toFloat(), 9.0f);
    g.setColour (palette.border);
    g.drawRoundedRectangle (board.toFloat(), 9.0f, 1.0f);
    g.setColour (palette.accentBright);
    g.setFont (juce::Font (10.5f, juce::Font::bold));
    g.drawText ("STRUM PATTERN", 25, boardY + 8, 105, 18, juce::Justification::centredLeft);
    static const int arrows[8] = { 0, 1, 0, 1, 0, 1, 0, 1 };
    for (int i = 0; i < 8; ++i)
    {
        const auto x = 34 + i * 16;
        const auto y = boardY + 41;
        g.setColour (palette.accent.withAlpha (0.8f));
        if (arrows[i] == 0)
        {
            g.drawLine (static_cast<float> (x), static_cast<float> (y - 9), static_cast<float> (x), static_cast<float> (y + 8), 2.0f);
            g.drawLine (static_cast<float> (x - 4), static_cast<float> (y + 3), static_cast<float> (x), static_cast<float> (y + 8), 2.0f);
            g.drawLine (static_cast<float> (x + 4), static_cast<float> (y + 3), static_cast<float> (x), static_cast<float> (y + 8), 2.0f);
        }
        else
        {
            g.drawLine (static_cast<float> (x), static_cast<float> (y + 8), static_cast<float> (x), static_cast<float> (y - 9), 2.0f);
            g.drawLine (static_cast<float> (x - 4), static_cast<float> (y - 4), static_cast<float> (x), static_cast<float> (y - 9), 2.0f);
            g.drawLine (static_cast<float> (x + 4), static_cast<float> (y - 4), static_cast<float> (x), static_cast<float> (y - 9), 2.0f);
        }
    }

    const auto fretStart = 185.0f;
    const auto fretEnd = static_cast<float> (getWidth() - 250);
    for (int string = 0; string < 6; ++string)
    {
        const auto y = static_cast<float> (boardY + 22 + string * 8);
        g.setColour (withAlpha (palette.muted, 0.72f));
        g.drawLine (fretStart, y, fretEnd, y, string == 0 ? 1.8f : 1.0f);
    }
    for (int fret = 0; fret <= 14; ++fret)
    {
        const auto x = fretStart + static_cast<float> (fret) * (fretEnd - fretStart) / 14.0f;
        g.setColour (withAlpha (palette.border, 0.9f));
        g.drawLine (x, static_cast<float> (boardY + 18), x, static_cast<float> (boardY + 66), 1.0f);
    }
    g.setColour (palette.muted);
    g.setFont (juce::Font (9.5f));
    g.drawText ("KEY", getWidth() - 213, boardY + 12, 35, 14, juce::Justification::centred);
    g.setColour (palette.text);
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.drawText ("C", getWidth() - 210, boardY + 34, 31, 22, juce::Justification::centred);
    g.setColour (palette.muted);
    g.setFont (juce::Font (9.5f));
    g.drawText ("TEMPO", getWidth() - 160, boardY + 12, 55, 14, juce::Justification::centred);
    g.setColour (palette.text);
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("DAW", getWidth() - 158, boardY + 34, 54, 22, juce::Justification::centred);
}

void WoodlineAudioProcessorEditor::resized()
{
    const auto width = getWidth();
    const auto height = getHeight();
    themeBox.setBounds (width - 148, 13, 132, 29);
    themeLabel.setBounds (width - 212, 17, 55, 20);
    presetBrowser.setBounds (12, 64, width - 24, 337);

    const int top = 412;
    const int bottom = height - 101;
    const int panelHeight = juce::jmax (220, bottom - top);
    const int gap = 8;
    const int innerWidth = width - 24 - gap * 4;
    const int widths[] = { innerWidth * 18 / 100, innerWidth * 14 / 100, innerWidth * 19 / 100,
                           innerWidth * 25 / 100, 0 };
    int x = 12;
    int remaining = innerWidth;
    for (int i = 0; i < 4; ++i)
    {
        const auto w = widths[i];
        panelBounds[static_cast<size_t> (i)] = { x, top, w, panelHeight };
        x += w + gap;
        remaining -= w;
    }
    panelBounds[4] = { x, top, remaining, panelHeight };

    const auto p0 = panelBounds[0].reduced (8, 48);
    const auto knobW0 = (p0.getWidth() - 8) / 2;
    placeKnob (0, { p0.getX(), p0.getY(), knobW0, 125 });
    placeKnob (1, { p0.getX() + knobW0 + 8, p0.getY(), knobW0, 125 });
    strumStyleLabel.setBounds (p0.getX(), p0.getY() + 128, p0.getWidth(), 16);
    strumStyleBox.setBounds (p0.getX(), p0.getY() + 146, p0.getWidth(), 29);
    latchButton.setBounds (p0.getX(), p0.getY() + 188, p0.getWidth(), 32);

    const auto p1 = panelBounds[1].reduced (9, 54);
    chordModeLabel.setBounds (p1.getX(), p1.getY() + 8, p1.getWidth(), 16);
    chordModeBox.setBounds (p1.getX(), p1.getY() + 28, p1.getWidth(), 31);
    voicingLabel.setBounds (p1.getX(), p1.getY() + 87, p1.getWidth(), 16);
    voicingBox.setBounds (p1.getX(), p1.getY() + 107, p1.getWidth(), 31);

    const auto p2 = panelBounds[2].reduced (9, 48);
    placeKnob (2, { p2.getX(), p2.getY(), 76, 124 });
    loopPatternLabel.setBounds (p2.getX() + 78, p2.getY() + 8, p2.getWidth() - 78, 16);
    loopPatternBox.setBounds (p2.getX() + 78, p2.getY() + 28, p2.getWidth() - 78, 31);
    loopButton.setBounds (p2.getX(), p2.getY() + 150, p2.getWidth(), 30);
    syncButton.setBounds (p2.getX(), p2.getY() + 188, p2.getWidth(), 30);

    const auto p3 = panelBounds[3].reduced (5, 50);
    const int toneW = p3.getWidth() / 6;
    for (int i = 0; i < 6; ++i)
        placeKnob (3 + i, { p3.getX() + i * toneW, p3.getY() + 14, toneW, 142 });

    const auto p4 = panelBounds[4].reduced (5, 50);
    const int fxW = p4.getWidth() / 5;
    for (int i = 0; i < 5; ++i)
        placeKnob (9 + i, { p4.getX() + i * fxW, p4.getY() + 14, fxW, 142 });
}

void WoodlineAudioProcessorEditor::timerCallback()
{
    displayedOutputLevel = processor.getOutputLevel();
    repaint (getWidth() - 180, 0, 180, 56);
}
