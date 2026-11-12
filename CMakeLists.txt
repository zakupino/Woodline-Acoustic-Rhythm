cmake_minimum_required(VERSION 3.22)

project(WoodlineAcousticRhythm VERSION 1.0.0 LANGUAGES C CXX)

include(FetchContent)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.5
    GIT_SHALLOW TRUE
    GIT_SUBMODULES_RECURSE TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Woodline
    COMPANY_NAME "Woodline Audio"
    BUNDLE_ID com.woodlineaudio.acousticrhythm
    PLUGIN_MANUFACTURER_CODE Wdln
    PLUGIN_CODE Wdar
    PRODUCT_NAME "Woodline Acoustic Rhythm"
    DESCRIPTION "Hybrid acoustic rhythm guitar instrument"
    VERSION 1.0.0
    FORMATS VST3
    VST3_CATEGORIES Instrument Synth
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(Woodline PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PresetLibrary.cpp
    Source/PresetLibrary.h
    Source/Theme.h
)

target_compile_features(Woodline PRIVATE cxx_std_17)

target_compile_definitions(Woodline PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(Woodline
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if (MSVC)
    target_compile_options(Woodline PRIVATE /W4 /permissive-)
else()
    target_compile_options(Woodline PRIVATE -Wall -Wextra -Wpedantic)
endif()
