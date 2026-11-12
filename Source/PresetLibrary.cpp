#include "PresetLibrary.h"

const std::vector<WoodlinePreset>& getWoodlinePresets()
{
    static const std::vector<WoodlinePreset> presets = []
    {
        const juce::String categories[] = { "Strumming", "Fingerstyle", "Pop & Ballad", "Rock", "Country", "Cinematic" };
        const juce::String names[6][10] =
        {
            { "Stadium Drive", "Steady Downstroke", "Open Road", "Campfire Eight", "Big Chorus", "Palm Muted Pulse", "Bright Sixteen", "Indie Motion", "Nashville Flow", "Acoustic Anthem" },
            { "Soft Arpeggio", "Morning Picking", "Nylon Pattern", "Broken Chords", "Gentle Thumb", "River Picking", "Dusty Waltz", "Intimate Verse", "Folk Arpeggio", "Cinematic Picking" },
            { "Warm Acoustic", "Sunday Ballad", "Acoustic Pop", "Soft Focus", "Heartline", "Radio Strum", "Sunset Verse", "Sweet Chorus", "Modern Folk Pop", "Piano Companion" },
            { "Rock Rhythm", "Driving Eight", "Arena Acoustic", "Gritty Strum", "Power Folk", "Wide Chorus", "Tight Attack", "Alt Rock Bed", "Big Open", "Pulse Rock" },
            { "Country Vibe", "Nashville Gentle", "Porch Swing", "Bluegrass Drive", "Dusty Highway", "Honky Tonk", "Wide Prairie", "Train Beat", "Southern Light", "Country Pop" },
            { "Chill Acoustic", "Golden Hour", "Ambient Bloom", "Wide Horizon", "Film Score Soft", "Dreamscape", "Slow Motion", "Emotional Rise", "Atmosphere", "Final Scene" }
        };

        // Hand-authored mappings to familiar acoustic accompaniment feels:
        // pop 8ths, ballad, straight folk, country boom-chuck, indie syncopation,
        // fingerpicked arpeggio, rock quarters and continuous 16ths.
        static constexpr int categoryPatternMap[6][10] =
        {
            { 0, 0, 2, 1, 6, 3, 7, 0, 3, 6 },
            { 5, 5, 5, 4, 5, 4, 5, 5, 4, 5 },
            { 0, 1, 0, 1, 4, 0, 1, 0, 4, 0 },
            { 6, 6, 7, 6, 7, 6, 7, 6, 6, 7 },
            { 3, 3, 4, 3, 4, 3, 3, 4, 3, 4 },
            { 5, 1, 5, 4, 5, 5, 1, 5, 4, 5 }
        };

        std::vector<WoodlinePreset> result;
        result.reserve (60);
        for (int category = 0; category < 6; ++category)
        {
            for (int i = 0; i < 10; ++i)
            {
                WoodlinePreset p;
                p.name = names[category][i];
                p.category = categories[category];
                p.strumSpeed = 0.72f + 0.12f * static_cast<float> (i % 6);
                p.humanize = 18.0f + static_cast<float> ((i * 9 + category * 5) % 55);
                p.pickAttack = 42.0f + static_cast<float> ((i * 7 + category * 8) % 48);
                p.body = 48.0f + static_cast<float> ((i * 6 + category * 7) % 45);
                p.bass = 42.0f + static_cast<float> ((i * 5 + category * 3) % 35);
                p.mid = 45.0f + static_cast<float> ((i * 4 + category * 6) % 30);
                p.treble = 48.0f + static_cast<float> ((i * 8 + category * 4) % 42);
                p.reverb = category == 5 ? 52.0f : category == 1 ? 30.0f : 18.0f + static_cast<float> (i * 2);
                p.delay = category == 5 ? 28.0f : category == 3 ? 12.0f : 5.0f + static_cast<float> (i % 5) * 3.0f;
                p.chorus = category == 5 ? 34.0f : category == 1 ? 12.0f : 4.0f + static_cast<float> (i % 4) * 4.0f;
                p.width = category == 5 ? 92.0f : 60.0f + static_cast<float> (i % 5) * 7.0f;
                p.strumStyle = (category == 1 || category == 5) ? 4
                             : (category == 3) ? 1
                             : (category == 2 || category == 4) ? 3 : 0;
                p.chordMode = (i % 7 == 4 ? 1 : i % 7 == 5 ? 2 : i % 7 == 6 ? 3 : 0);
                p.voicing = (i % 6 == 5 ? 2 : i % 6 == 3 ? 1 : 0);
                p.loopPattern = categoryPatternMap[category][i];
                p.description = category == 0 ? "Punchy steel-string strumming with a natural pick attack." :
                                category == 1 ? "Soft picked notes with warm sustain and intimate detail." :
                                category == 2 ? "Balanced modern acoustic tone for pop and ballad arrangements." :
                                category == 3 ? "Focused attack and a confident rhythmic acoustic layer." :
                                category == 4 ? "Bright, woody country rhythm with a lively groove." :
                                                "Wide, spacious acoustic texture for cinematic arrangements.";
                result.push_back (p);
            }
        }
        return result;
    }();
    return presets;
}
