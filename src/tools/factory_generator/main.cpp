#include <iostream>
#include <juce_core/juce_core.h>
#include "FactoryData.h"

void writeAssets(const juce::String& subDir, const std::vector<FactoryAsset>& assets, const juce::String& outputRoot) {
    juce::File dir = juce::File(outputRoot).getChildFile(subDir);
    dir.createDirectory();

    for (const auto& asset : assets) {
        juce::File filePath = dir.getChildFile(asset.filename);
        if (filePath.replaceWithText(asset.content)) {
            std::cout << "Generated: " << filePath.getFullPathName() << std::endl;
        } else {
            std::cerr << "Error: Could not write to " << filePath.getFullPathName() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: FactoryGenerator <output_directory>" << std::endl;
        return 1;
    }

    juce::String outputDir = argv[1];
    
    try {
        writeAssets("Themes", FactoryData::getAllThemes(), outputDir);
        writeAssets("Envelopes", FactoryData::getAllEnvelopes(), outputDir);
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
