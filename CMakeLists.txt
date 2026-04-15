cmake_minimum_required(VERSION 3.22)

project(TruckPackerWrapper VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
)
FetchContent_MakeAvailable(JUCE)

set(_tp_formats VST3 Standalone)
if(APPLE)
    list(APPEND _tp_formats AU)
endif()

juce_add_plugin(TruckPackerWrapper
    COMPANY_NAME "Truck Packer"
    BUNDLE_ID com.truckpacker.dawwrapper
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Tcpk
    PLUGIN_CODE TpWr
    FORMATS ${_tp_formats}
    PRODUCT_NAME "Truck Packer"
    MICROPHONE_PERMISSION_ENABLED FALSE
    MICROPHONE_PERMISSION_TEXT ""
)

target_sources(TruckPackerWrapper
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/TruckPackerWebView.cpp
)

target_compile_definitions(TruckPackerWrapper
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

if(WIN32)
    target_compile_definitions(TruckPackerWrapper
        PUBLIC
            JUCE_USE_WIN_WEBVIEW2=1
    )
endif()

target_link_libraries(TruckPackerWrapper
    PRIVATE
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Copy bundles next to the repo (macOS AU + VST3).
set(_tp_stage "${CMAKE_SOURCE_DIR}/packaged-plugins")
add_custom_command(
    TARGET TruckPackerWrapper_VST3
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_tp_stage}"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${_tp_stage}/Truck Packer.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:TruckPackerWrapper_VST3>" "${_tp_stage}/Truck Packer.vst3"
    VERBATIM)

if(APPLE)
    add_custom_command(
        TARGET TruckPackerWrapper_AU
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_tp_stage}"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${_tp_stage}/Truck Packer.component"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:TruckPackerWrapper_AU>" "${_tp_stage}/Truck Packer.component"
        VERBATIM)
endif()

# -----------------------------------------------------------------------------
# Local packaging (see packaging/build_mac_pkg.sh and packaging/build_win_installer.ps1)
# -----------------------------------------------------------------------------
add_custom_target(stage_plugins_for_package
    COMMAND ${CMAKE_COMMAND}
        -DTP_BUILD_DIR=${CMAKE_BINARY_DIR}
        -DTP_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DTP_CONFIG=$<CONFIG>
        -P ${CMAKE_SOURCE_DIR}/packaging/stage_plugins.cmake
    DEPENDS TruckPackerWrapper_VST3
    COMMENT "Staging plug-ins under packaging/_stage for installers")

if(APPLE)
    add_dependencies(stage_plugins_for_package TruckPackerWrapper_AU)
endif()
