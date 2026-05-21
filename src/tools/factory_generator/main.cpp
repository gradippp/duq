#include <iostream>
#include <fstream>
#include <filesystem>
#include "FactoryData.h"

namespace fs = std::filesystem;

void writeAssets(const std::string& subDir, const std::vector<FactoryAsset>& assets, const std::string& outputRoot) {
    fs::path dir = fs::path(outputRoot) / subDir;
    fs::create_directories(dir);

    for (const auto& asset : assets) {
        fs::path filePath = dir / asset.filename;
        std::ofstream out(filePath);
        if (out.is_open()) {
            out << asset.content;
            out.close();
            std::cout << "Generated: " << filePath.string() << std::endl;
        } else {
            std::cerr << "Error: Could not write to " << filePath.string() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: FactoryGenerator <output_directory>" << std::endl;
        return 1;
    }

    std::string outputDir = argv[1];
    
    try {
        writeAssets("themes", FactoryData::getAllThemes(), outputDir);
        writeAssets("envelopes", FactoryData::getAllEnvelopes(), outputDir);
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
