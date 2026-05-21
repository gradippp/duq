cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "Runtime Library")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "Build for 10.13")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# macOS Universal Binary support (Intel + Apple Silicon)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build for both Silicon and Intel")
endif()

option(BUILD_STANDALONE "Build Standalone plugin format" ON)
option(BUILD_VST3 "Build VST3 plugin format" ON)
option(BUILD_LV2 "Build LV2 plugin format" OFF)

if(APPLE)
    option(BUILD_AU "Build AU plugin format" ON)
else()
    set(BUILD_AU OFF)
endif()

if(NOT DEFINED PLUGIN_VERSION)
    set(PLUGIN_VERSION "0.0.0")
endif()

if(NOT DEFINED FULL_PLUGIN_VERSION)
    set(FULL_PLUGIN_VERSION "${PLUGIN_VERSION}")
endif()

set(PLUGIN_NAME Duq)
project(${PLUGIN_NAME} VERSION ${PLUGIN_VERSION})

# Build Format List
set(PLUGIN_FORMATS "")
if(BUILD_VST3)
    list(APPEND PLUGIN_FORMATS VST3)
endif()
if(BUILD_STANDALONE)
    list(APPEND PLUGIN_FORMATS Standalone)
endif()
if(BUILD_AU)
    list(APPEND PLUGIN_FORMATS AU)
endif()
if(BUILD_LV2)
    list(APPEND PLUGIN_FORMATS LV2)
endif()

# Get the current git hash
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE GIT_RESULT
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT GIT_RESULT EQUAL 0)
    set(GIT_COMMIT "unknown")
endif()

set(PROJECT_URI "https://agradip.fyi/duq")
set(PROJECT_GITHUB_URL "https://github.com/gradippp/duq")

add_compile_definitions(
    PROJECT_VERSION="${FULL_PLUGIN_VERSION}"
    PROJECT_GIT_COMMIT="${GIT_COMMIT}"
    PROJECT_URI="${PROJECT_URI}"
    PROJECT_GITHUB_URL="${PROJECT_GITHUB_URL}"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(external/JUCE)

juce_add_plugin(${PLUGIN_NAME}
        COMPANY_NAME ${PLUGIN_NAME}
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Agra
        PLUGIN_CODE duq1
        FORMATS ${PLUGIN_FORMATS}
        PRODUCT_NAME ${PLUGIN_NAME}
        BUNDLE_ID "fyi.agradip.duq"
        LV2URI "${PROJECT_URI}"
)

file(GLOB_RECURSE src
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Exclude tools from the main plugin target
list(FILTER src EXCLUDE REGEX "src/tools/.*")

target_sources(${PLUGIN_NAME}
    PRIVATE
        ${src}
)


source_group(Source
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginProcessor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginEditor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginEditor.cpp
)

file(GLOB UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.cpp
)
source_group(Source\\ui FILES ${UI_SOURCES})

file(GLOB_RECURSE DUQ_RESOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/res/*
)

juce_add_binary_data(${PLUGIN_NAME}BinaryData
    SOURCES ${DUQ_RESOURCES}
)

target_link_libraries(${PLUGIN_NAME} PRIVATE ${PLUGIN_NAME}BinaryData)

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PLUGIN_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# JUCE libraries to bring into our project
juce_generate_juce_header(${PLUGIN_NAME})
target_link_libraries(${PLUGIN_NAME}
    PRIVATE
        juce::juce_dsp
        juce::juce_core
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

set_target_properties(${PLUGIN_NAME} PROPERTIES FOLDER "Shared Code")
if (TARGET ${PLUGIN_NAME}_VST3)
    set_target_properties(${PLUGIN_NAME}_VST3 PROPERTIES FOLDER "Wrappers")
endif()
if (TARGET ${PLUGIN_NAME}_Standalone)
    set_target_properties(${PLUGIN_NAME}_Standalone PROPERTIES FOLDER "Wrappers")
endif()
if (TARGET ${PLUGIN_NAME}_AU)
    set_target_properties(${PLUGIN_NAME}_AU PROPERTIES FOLDER "Wrappers")
endif()
if (TARGET ${PLUGIN_NAME}_LV2)
    set_target_properties(${PLUGIN_NAME}_LV2 PROPERTIES FOLDER "Wrappers")
endif()

if (TARGET ${PLUGIN_NAME}_VST3)
    message(STATUS "${PLUGIN_NAME}_VST3 target created successfully")
endif()

# --- Factory Content Generation ---
file(GLOB_RECURSE GENERATOR_SRCS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/factory_generator/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/factory_generator/*.h"
)

# We also need these core plugin logic files for the generator
list(APPEND GENERATOR_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/utils/ThemeManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/utils/FontManager.cpp"
)

add_executable(FactoryGenerator ${GENERATOR_SRCS})
target_include_directories(FactoryGenerator PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/factory_generator"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_${PLUGIN_NAME}BinaryData/JuceLibraryCode"
)

# Link to JUCE modules needed for serialization
target_link_libraries(FactoryGenerator PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_events
)

# Ensure BinaryData is generated before compiling FontManager
add_dependencies(FactoryGenerator ${PLUGIN_NAME}BinaryData)

set_target_properties(FactoryGenerator PROPERTIES FOLDER "Tools")

# FactoryGenerator uses std::filesystem, which requires macOS 10.15+
if(APPLE)
    set_target_properties(FactoryGenerator PROPERTIES XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "10.15")
endif()

set(FACTORY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/FactoryContent")

add_custom_command(
    OUTPUT "${FACTORY_OUTPUT_DIR}/themes" "${FACTORY_OUTPUT_DIR}/envelopes"
    COMMAND FactoryGenerator "${FACTORY_OUTPUT_DIR}"
    DEPENDS FactoryGenerator
    COMMENT "Generating Factory Themes and Envelopes from source..."
)

add_custom_target(GenerateFactoryContent ALL 
    DEPENDS "${FACTORY_OUTPUT_DIR}/themes" "${FACTORY_OUTPUT_DIR}/envelopes"
)
