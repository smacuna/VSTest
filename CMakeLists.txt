cmake_minimum_required(VERSION 3.15)

project(MySynth VERSION 0.0.1)

# Fetch JUCE 8
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(juce)

# Standard C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# Plugin Format & Config
juce_add_plugin(MySynth
    company_name "DeepmindAgent"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE Dem0
    FORMATS VST3 Standalone
    PRODUCT_NAME "MySynth"
)

# Generate JuceHeader.h for convenience and compatibility with source code
juce_generate_juce_header(MySynth)

target_sources(MySynth
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/SynthVoice.cpp
        Source/SynthVoice.h
)

target_compile_features(MySynth PUBLIC cxx_std_20)

target_link_libraries(MySynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_audio_processors 
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(MySynth PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)
