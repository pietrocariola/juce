# ==============================================================
#   FILE WITH BUILD CONFIGURATIONS
#   Docs: https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md   
#
#   1) go to the place where your code is
#   cd /path/to/BasicTemplate
#
#   2) prepare to build into "build" folder 
#   cmake -B build
#
#   3) build and compile the files in your "build" folder
#   cmake --build build
#
# ==============================================================

# pietro: cmake required version
cmake_minimum_required(VERSION 3.22)

# pietro: plugin's name and version
project(BasicTemplate VERSION 0.0.1)

# pietro: adds the path where JUCE is installed
list(APPEND CMAKE_PREFIX_PATH "C:/JUCE/install")

# pietro: adds JUCE dependencies
find_package(JUCE CONFIG REQUIRED)

# juce_add_plugin(<target> [KEY value]...)
# <target> is the target plugin BasicTemplate
# [KEY value] is a static library that takes into account 
# a KEY followed by one or more values
juce_add_plugin(BasicTemplate
    COPY_PLUGIN_AFTER_BUILD TRUE    # Copies plugin to default folder after build
                                    # MacOs: ~/Library/Audio/Plug-Ins/VST3
                                    # Linux: ~/.vst3
                                    # Windows: C:\Program Files\Common Files\VST3
    
    FORMATS VST3 Standalone         # Build plugin in VST3 and Standalone formats
    PRODUCT_NAME "BasicTemplate"    # Plugin's name
    COMPANY_NAME "Cariola_Project"  # Plugin's company name
)

# Minimum C++ version
set_target_properties(BasicTemplate PROPERTIES CXX_STANDARD 17)  

# List of plugin's .cpp files. Headers (files .h) enter inside the .cpp files
target_sources(BasicTemplate
    PRIVATE
        PluginProcessor.cpp
        PluginEditor.cpp
)

# Compile configurations
target_compile_definitions(BasicTemplate
    PUBLIC
        # Tells JUCE's VST wrapper to compile VST2 and VST3 separately
        JUCE_VST3_CAN_REPLACE_VST2=0)

# Libraries linked to target plugin 
target_link_libraries(BasicTemplate
    PRIVATE
        # JUCE's basic audio library
        juce::juce_audio_utils
    PUBLIC
        # add JUCE's configuration, link-time optimization, and warning flags
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
