cmake_minimum_required(VERSION 3.24)

project(AceStepPlugin VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(ACESTEP_PLUGIN_MODE "static" CACHE STRING "ACE-Step integration mode: static or server")
set_property(CACHE ACESTEP_PLUGIN_MODE PROPERTY STRINGS static server)
option(ACESTEP_ENABLE_ACESTEP_CPP "Configure and link the acestep.cpp backend" ON)
option(ACESTEP_BUILD_TESTS "Build focused plugin unit tests" OFF)

if(NOT ACESTEP_PLUGIN_MODE STREQUAL "static" AND NOT ACESTEP_PLUGIN_MODE STREQUAL "server")
    message(FATAL_ERROR "ACESTEP_PLUGIN_MODE must be either 'static' or 'server'.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerWarnings)

set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/JUCE")
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Vendored JUCE source is missing from External/JUCE.")
endif()

add_subdirectory("${JUCE_DIR}" EXCLUDE_FROM_ALL)

juce_add_plugin(AceStepPlugin
    BUNDLE_ID "com.allwavemedia.acestep"
    COMPANY_NAME "Allwave Media"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Acst
    PLUGIN_CODE Acsg
    FORMATS VST3
    PRODUCT_NAME "ACE-Step"
)

set(ACESTEP_PLUGIN_SOURCES
    Source/Audio/ReferenceAudioBuffer.cpp
    Source/Audio/ReferenceAudioBuffer.h
    Source/Audio/ReferenceAudioBufferSnapshot.cpp
    Source/Audio/ReferenceAudioBufferWriter.cpp
    Source/Audio/ReferenceAudioStorage.cpp
    Source/Audio/ReferenceAudioStorage.h
    Source/Engine/AceStepCApi.cpp
    Source/Engine/AceStepCApi.h
    Source/Engine/AceStepEngine.cpp
    Source/Engine/AceStepEngine.h
    Source/Engine/GenerationRequest.h
    Source/Engine/ReferenceAudioResampler.cpp
    Source/Engine/ReferenceAudioResampler.h
    Source/Models/GeneratedAssetHistory.cpp
    Source/Models/GeneratedAssetHistory.h
    Source/Models/ModelChecksum.cpp
    Source/Models/ModelChecksum.h
    Source/Models/ModelDiscovery.cpp
    Source/Models/ModelDiscovery.h
    Source/Models/ModelDownloader.cpp
    Source/Models/ModelDownloader.h
    Source/Models/ModelSetupState.cpp
    Source/Models/ModelSetupState.h
    Source/UI/ExternalFileDrag.cpp
    Source/UI/ExternalFileDrag.h
    Source/UI/GeneratedAssetTile.cpp
    Source/UI/GeneratedAssetTile.h
    Source/UI/PreviewPlayer.cpp
    Source/UI/PreviewPlayer.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

target_sources(AceStepPlugin PRIVATE ${ACESTEP_PLUGIN_SOURCES})

target_compile_definitions(AceStepPlugin
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
        JUCE_REPORT_APP_USAGE=0
        JUCE_USE_CURL=1
        JUCE_WEB_BROWSER=0
)

target_link_libraries(AceStepPlugin
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_gui_extra
)

target_apply_compiler_warnings(AceStepPlugin)

if(ACESTEP_ENABLE_ACESTEP_CPP)
    include(AcestepIntegration)
    acestep_configure(AceStepPlugin)
else()
    target_compile_definitions(AceStepPlugin PRIVATE ACESTEP_PLUGIN_MODE_STUB=1)
endif()

if(ACESTEP_ENABLE_ACESTEP_CPP)
    include(BundleBackends)
    acestep_bundle_backends(AceStepPlugin_VST3)
endif()

if(TARGET AceStepPlugin_VST3)
    set_target_properties(AceStepPlugin_VST3 PROPERTIES FOLDER "Plugin")
endif()

if(ACESTEP_BUILD_TESTS)
    enable_testing()

    juce_add_console_app(AceStepPluginTests
        PRODUCT_NAME "ACE-Step Plugin Tests"
    )

    target_sources(AceStepPluginTests
        PRIVATE
            Source/Audio/ReferenceAudioBuffer.cpp
            Source/Audio/ReferenceAudioBuffer.h
            Source/Audio/ReferenceAudioBufferSnapshot.cpp
            Source/Audio/ReferenceAudioBufferWriter.cpp
            Source/Audio/ReferenceAudioStorage.cpp
            Source/Audio/ReferenceAudioStorage.h
            Source/Engine/AceStepCApi.cpp
            Source/Engine/AceStepCApi.h
            Source/Engine/AceStepEngine.cpp
            Source/Engine/AceStepEngine.h
            Source/Engine/GenerationRequest.h
            Source/Engine/ReferenceAudioResampler.cpp
            Source/Engine/ReferenceAudioResampler.h
            Source/Models/GeneratedAssetHistory.cpp
            Source/Models/GeneratedAssetHistory.h
            Source/Models/ModelChecksum.cpp
            Source/Models/ModelChecksum.h
            Source/Models/ModelDiscovery.cpp
            Source/Models/ModelDiscovery.h
            Source/Models/ModelDownloader.cpp
            Source/Models/ModelDownloader.h
            Source/Models/ModelSetupState.cpp
            Source/Models/ModelSetupState.h
            Source/UI/ExternalFileDrag.cpp
            Source/UI/ExternalFileDrag.h
            Source/UI/GeneratedAssetTile.cpp
            Source/UI/GeneratedAssetTile.h
            Source/UI/PreviewPlayer.cpp
            Source/UI/PreviewPlayer.h
            Tests/AceStepCApi_test.cpp
            Tests/AceStepEngine_test.cpp
            Tests/AceStepEngineAsync_test.cpp
            Tests/AceStepEngineMock_test.cpp
            Tests/AceStepEngineProgress_test.cpp
            Tests/ExternalFileDrag_test.cpp
            Tests/GeneratedAssetHistory_test.cpp
            Tests/GeneratedAssetTile_test.cpp
            Tests/GenerationTypes_test.cpp
            Tests/ModelChecksum_test.cpp
            Tests/ModelDiscovery_test.cpp
            Tests/ModelDownloader_test.cpp
            Tests/ModelDownloaderErrors_test.cpp
            Tests/ModelSetupState_test.cpp
            Tests/PreviewPlayer_test.cpp
            Tests/ReferenceAudioBufferBasic_test.cpp
            Tests/ReferenceAudioBufferConcurrent_test.cpp
            Tests/ReferenceAudioResampler_test.cpp
            Tests/ReferenceAudioBufferTestMain.cpp
            Tests/ReferenceAudioBufferTestUtils.h
    )

    target_compile_definitions(AceStepPluginTests
        PRIVATE
            JUCE_DISPLAY_SPLASH_SCREEN=0
            JUCE_REPORT_APP_USAGE=0
            ACESTEP_PLUGIN_MODE_STUB=1
    )

    target_link_libraries(AceStepPluginTests
        PRIVATE
            juce::juce_audio_basics
            juce::juce_audio_devices
            juce::juce_audio_formats
            juce::juce_audio_utils
            juce::juce_core
            juce::juce_gui_basics
    )

    target_apply_compiler_warnings(AceStepPluginTests)

    add_test(NAME ReferenceAudioBuffer COMMAND AceStepPluginTests)
endif()
