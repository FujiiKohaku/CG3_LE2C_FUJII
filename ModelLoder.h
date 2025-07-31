#pragma once
#include "CommonStructs.h" // ModelData, MaterialDataを使う
#include <filesystem>
#include <string>
// objファイルとmtlファイルを読み込む関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

// mtlファイルだけを読む関数
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
ModelData LoadObjFileNoTexture(const std::filesystem::path& directoryPath, const std::string& filename);