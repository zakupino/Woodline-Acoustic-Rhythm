# Implementation status — version 1.0.0 source prototype

## Included now

- JUCE VST3 instrument target and GitHub Actions build workflow.
- Windows x64, macOS ARM64 and macOS Intel x64 build definitions.
- Karplus–Strong plucked-string DSP engine with pick transient/noise excitation and body coloration.
- Six one-note chord types and three voicing modes.
- MIDI-triggered strumming and eight DAW-synced rhythm patterns.
- Latch and C1–B1 key-switch handling.
- 60 parameter presets, search, category filters, BPM sorting and preset loading.
- Six switchable interface themes.
- APVTS parameter automation and DAW project state save/restore.

## Not included yet — do not treat this source prototype as a finished commercial instrument

- Recorded multi-sampled acoustic guitar audio, velocity layers and round robins.
- A 50+ individually authored rhythm-loop library. Eight starter patterns are implemented in this version.
- Full recorded fret slides, releases, palm mutes and detailed performance articulations.
- In-plugin persistent user-preset save/import/export. Current parameter settings are saved by the DAW project.
- Installer, code-signing, notarization or a finished commercial product release pipeline.
- A successful local binary build from this environment. The source build must be compiled by GitHub Actions and then tested in Ableton Live.

These are separate development tasks, not hidden dependencies. The architecture is deliberately organized so licensed sample content and additional rhythm patterns can be added in later versions.
