cmake_minimum_required(VERSION 3.24)
project(FreePitch VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/arch:AVX2 /W4 /MP /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Prefer local shallow-cloned JUCE if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# Main FreePitch VST3 and Standalone targets
juce_add_plugin(FreePitch
    COMPANY_NAME "FreePitch Audio"
    BUNDLE_ID "com.freepitchaudio.freepitch"
    PLUGIN_MANUFACTURER_CODE FPch
    PLUGIN_CODE FP01
    FORMATS VST3 Standalone
    PRODUCT_NAME "FreePitch"
    DESCRIPTION "Professional Low-Latency Real-Time Pitch Correction & Vocal EFX"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_include_directories(FreePitch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/UI
)

file(GLOB_RECURSE FreePitch_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)

target_sources(FreePitch PRIVATE ${FreePitch_SOURCES})

target_compile_definitions(FreePitch PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(FreePitch PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Automated DSP Test Suite Target
add_executable(FreePitchDSPTests
    Tests/DSPUnitTests.cpp
    Source/DSP/PitchDetector.cpp
    Source/DSP/VoicedClassifier.cpp
    Source/DSP/ScaleQuantizer.cpp
    Source/DSP/PitchShifter.cpp
    Source/DSP/FormantEngine.cpp
    Source/DSP/EFXRack.cpp
)

target_include_directories(FreePitchDSPTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP
)

target_compile_definitions(FreePitchDSPTests PRIVATE
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
)

target_link_libraries(FreePitchDSPTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_graphics
    juce::juce_dsp
    juce::juce_core
)
