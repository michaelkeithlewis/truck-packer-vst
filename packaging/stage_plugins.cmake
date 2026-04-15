# Stage built JUCE plug-ins for installers (run with: cmake -P stage_plugins.cmake)
# Required: -DTP_BUILD_DIR=.../build  -DTP_SOURCE_DIR=.../repo  -DTP_CONFIG=Release|Debug
if(NOT DEFINED TP_BUILD_DIR OR NOT DEFINED TP_SOURCE_DIR)
    message(FATAL_ERROR "Usage: cmake -DTP_BUILD_DIR=<build> -DTP_SOURCE_DIR=<source> [-DTP_CONFIG=Release] -P stage_plugins.cmake")
endif()

if(NOT DEFINED TP_CONFIG)
    set(TP_CONFIG Release)
endif()

set(_art "${TP_BUILD_DIR}/TruckPackerWrapper_artefacts/${TP_CONFIG}")
set(_out "${TP_SOURCE_DIR}/packaging/_stage")

message(STATUS "Staging from ${_art} -> ${_out}")

if(NOT EXISTS "${_art}/VST3/Truck Packer.vst3")
    message(FATAL_ERROR "VST3 bundle not found: ${_art}/VST3/Truck Packer.vst3 (build ${TP_CONFIG} first)")
endif()

file(REMOVE_RECURSE "${_out}")
file(MAKE_DIRECTORY "${_out}")

# cmake -P does not define APPLE/WIN32; use host variables.
if(CMAKE_HOST_APPLE)
    file(MAKE_DIRECTORY "${_out}/Library/Audio/Plug-Ins/VST3")
    file(MAKE_DIRECTORY "${_out}/Library/Audio/Plug-Ins/Components")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${_art}/VST3/Truck Packer.vst3"
        "${_out}/Library/Audio/Plug-Ins/VST3/Truck Packer.vst3")
    if(EXISTS "${_art}/AU/Truck Packer.component")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${_art}/AU/Truck Packer.component"
            "${_out}/Library/Audio/Plug-Ins/Components/Truck Packer.component")
    endif()
else()
    file(MAKE_DIRECTORY "${_out}/VST3")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${_art}/VST3/Truck Packer.vst3"
        "${_out}/VST3/Truck Packer.vst3")
endif()

file(COPY "${TP_SOURCE_DIR}/packaging/INSTALL.txt" DESTINATION "${_out}")
message(STATUS "Staged OK: ${_out}")
