# Woodline Acoustic Rhythm — VST3 source project

A JUCE/C++ instrument project for a MIDI-triggered acoustic rhythm guitar. The repository is configured to build a VST3 plug-in through GitHub Actions on Windows x64, macOS ARM64 and macOS Intel x64.

> **Important audio note:** this first build uses a Karplus–Strong plucked-string physical-model engine, transient excitation, body coloration, stereo delay and reverb. It is a playable prototype and architecture foundation, not yet a multisampled recording of a real acoustic guitar. The project does not include commercial guitar recordings. To reach a truly studio-realistic hybrid sound, add properly licensed steel-string/nylon-string multisamples and round-robin layers in a later content-engine phase.

## Features in this source build

- VST3 instrument with MIDI input and stereo output.
- One-note chord triggering with selectable Major, Minor, 7th, Maj7, Sus2 and Power chord modes.
- Six-note strums, Down/Up/Alternate/Fingerpick style selection, strum speed and humanization.
- Eight rhythm-loop patterns, loop latch, key-switch pattern changes and Ableton/DAW tempo sync.
- 60 built-in parameter presets across Strumming, Fingerstyle, Pop & Ballad, Rock, Country and Cinematic categories.
- Custom preset browser with search, categories, preset details and one-click loading.
- Six UI themes: Studio Dark, Soft Light, Ocean Blue, Forest Green, Aurora Violet and Ember Gradient.
- Host automation/state recall through JUCE AudioProcessorValueTreeState.

## Build it using GitHub Actions (no local compiler installation)

1. Create a **Public** or **Private** repository on GitHub. A name such as `Woodline-Acoustic-Rhythm` is fine.
2. Upload the contents of this project folder to the repository root. Keep the `.github/workflows/build-vst3.yml` path exactly as it is. Do not upload the outer ZIP as a single file; GitHub Actions needs the extracted project files and folder structure.
3. Open the repository's **Actions** tab.
4. Select **Build Woodline VST3** and open the latest run. The first build downloads JUCE 9.0.5 and compiles the plug-in.
5. When the Windows and Mac build jobs finish successfully, open the run and download the artifact named `Woodline-Windows-x64-VST3` (for Windows) or `Woodline-macOS-ARM64-VST3` or `Woodline-macOS-Intel-x64-VST3` (for Mac).
6. Extract the artifact ZIP. It contains the `Woodline Acoustic Rhythm.vst3` plug-in bundle/folder.

## Install the built VST3 in Ableton Live

### Windows

Copy the entire `Woodline Acoustic Rhythm.vst3` folder into:

```text
C:\Program Files\Common Files\VST3
```

Open Ableton Live → Settings/Preferences → Plug-ins. Make sure **Use VST3 Plug-In System Folders** is enabled, then run **Rescan**. Find it under Plug-ins → VST3 → Woodline Audio.

### macOS

Copy the entire `Woodline Acoustic Rhythm.vst3` bundle into:

```text
~/Library/Audio/Plug-Ins/VST3/
```

Then reopen Ableton Live or rescan plug-ins in Settings/Preferences.

There is no Windows installer in this project. The VST3 bundle must still be copied to a VST3 plug-in location so the DAW can discover it; this is a one-time file copy, not a separate application installation.

## MIDI map

- Play any note above the key-switch range to trigger a chord based on that note's pitch class.
- MIDI notes **24–35** (C1–B1 in the common MIDI note-number convention) select rhythm patterns while a chord loop is active. The first eight notes select patterns 1–8; the remaining four cycle back through patterns 1–4.
- Hold a chord note to keep the rhythm running. Turn **Latch** on to continue after note release.
- Turn **DAW Sync** on to follow the host tempo. Turn it off to use the plug-in Tempo control.
- The Pattern menu in the UI selects the same eight built-in patterns.

Some DAWs label MIDI octaves differently. If the visible octave name differs, use MIDI note numbers 24–35 for the key switches.

## Themes

The Theme menu in the upper-right changes the plug-in UI palette. The selected theme is stored in the plug-in state and recalled with the DAW project.

## Project structure

```text
Woodline_Acoustic_Rhythm_VST/
├── .github/workflows/build-vst3.yml
├── Source/
│   ├── PluginProcessor.h/.cpp
│   ├── PluginEditor.h/.cpp
│   ├── PresetLibrary.h/.cpp
│   └── Theme.h
├── CMakeLists.txt
├── README.md
└── .gitignore
```

## Dependency and build information

- Framework: [JUCE](https://github.com/juce-framework/JUCE), pinned to **9.0.5** in `CMakeLists.txt`.
- Build system: CMake 3.22+.
- Windows runner: GitHub-hosted Windows Server 2022 with Visual Studio 2022, x64.
- macOS runner: GitHub-hosted macOS ARM64 and Intel x86_64 runners (separate artifacts).
- Plug-in format: VST3 only. VST2 `.dll` is not built.

JUCE is an external dependency fetched by CMake during the GitHub Actions build. Review JUCE's current licensing terms for your distribution model before publishing a commercial product.

## Release checklist before selling or distributing

- Test the VST3 in Ableton Live on the exact Windows/macOS versions you support.
- Add licensed multisampled guitar recordings (multiple velocity layers, round robins, release/fret/pick noises) for realism.
- Profile CPU use with long sessions and high polyphony.
- Implement and test user-preset saving, preset import/export and a proper sample-content updater.
- Code-sign/notarize macOS distributions if required by your distribution channel.
