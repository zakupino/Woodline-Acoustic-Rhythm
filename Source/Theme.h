#pragma once
#include <JuceHeader.h>

struct WoodlinePalette
{
    juce::Colour background;
    juce::Colour panel;
    juce::Colour panelRaised;
    juce::Colour card;
    juce::Colour text;
    juce::Colour muted;
    juce::Colour accent;
    juce::Colour accentBright;
    juce::Colour border;
    juce::Colour field;
};

inline WoodlinePalette getWoodlinePalette (int themeIndex)
{
    using juce::Colour;
    switch (themeIndex)
    {
        case 1: // Soft light
            return { Colour::fromRGB (228, 232, 235), Colour::fromRGB (244, 246, 247),
                     Colour::fromRGB (255, 255, 255), Colour::fromRGB (235, 239, 242),
                     Colour::fromRGB (32, 42, 52), Colour::fromRGB (104, 116, 127),
                     Colour::fromRGB (28, 137, 190), Colour::fromRGB (48, 173, 222),
                     Colour::fromRGB (195, 204, 211), Colour::fromRGB (250, 252, 253) };
        case 2: // Ocean blue
            return { Colour::fromRGB (10, 22, 35), Colour::fromRGB (17, 34, 51),
                     Colour::fromRGB (24, 46, 66), Colour::fromRGB (20, 43, 62),
                     Colour::fromRGB (229, 242, 250), Colour::fromRGB (143, 170, 190),
                     Colour::fromRGB (0, 174, 220), Colour::fromRGB (87, 218, 247),
                     Colour::fromRGB (48, 79, 101), Colour::fromRGB (12, 29, 44) };
        case 3: // Forest green
            return { Colour::fromRGB (17, 27, 27), Colour::fromRGB (25, 39, 37),
                     Colour::fromRGB (34, 51, 47), Colour::fromRGB (29, 48, 42),
                     Colour::fromRGB (230, 240, 232), Colour::fromRGB (151, 174, 158),
                     Colour::fromRGB (71, 183, 131), Colour::fromRGB (118, 220, 165),
                     Colour::fromRGB (58, 86, 73), Colour::fromRGB (19, 34, 30) };
        case 4: // Aurora violet
            return { Colour::fromRGB (24, 20, 37), Colour::fromRGB (34, 29, 51),
                     Colour::fromRGB (48, 39, 70), Colour::fromRGB (39, 32, 61),
                     Colour::fromRGB (243, 238, 252), Colour::fromRGB (174, 160, 197),
                     Colour::fromRGB (164, 115, 239), Colour::fromRGB (207, 163, 255),
                     Colour::fromRGB (81, 67, 109), Colour::fromRGB (28, 24, 44) };
        case 5: // Ember gradient
            return { Colour::fromRGB (31, 24, 23), Colour::fromRGB (46, 34, 32),
                     Colour::fromRGB (61, 43, 39), Colour::fromRGB (49, 35, 32),
                     Colour::fromRGB (250, 239, 232), Colour::fromRGB (191, 157, 145),
                     Colour::fromRGB (226, 112, 83), Colour::fromRGB (255, 163, 118),
                     Colour::fromRGB (103, 68, 59), Colour::fromRGB (37, 28, 27) };
        case 0:
        default: // Studio dark: reference palette
            return { Colour::fromRGB (17, 20, 24), Colour::fromRGB (27, 32, 38),
                     Colour::fromRGB (37, 44, 52), Colour::fromRGB (31, 39, 47),
                     Colour::fromRGB (231, 237, 242), Colour::fromRGB (151, 163, 174),
                     Colour::fromRGB (35, 166, 231), Colour::fromRGB (81, 196, 250),
                     Colour::fromRGB (61, 72, 82), Colour::fromRGB (20, 25, 30) };
    }
}
