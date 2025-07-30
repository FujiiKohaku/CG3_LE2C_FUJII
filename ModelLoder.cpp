#include "ModelLoder.h"
#include "Utility.h"
#include <cassert>
#include <fstream>
#include <sstream>
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath,
    const std::string& filename)
{
    MaterialData materialData;
    std::string line;
    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            materialData.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }
    return materialData;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
    ModelData modelData;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string line;

    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.x *= -1.0f; // 右手系→左手系（位置）
            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y; // Y軸反転（DirectX用）
            texcoords.push_back(texcoord);
        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1.0f; // 右手系→左手系（法線）
            normals.push_back(normal);
        } else if (identifier == "f") {
            VertexData triangle[3];
            for (int i = 0; i < 3; ++i) {
                std::string vertexDefinition;
                s >> vertexDefinition;

                std::istringstream v(vertexDefinition);
                uint32_t idx[3] = {};
                for (int e = 0; e < 3; ++e) {
                    std::string index;
                    std::getline(v, index, '/');
                    idx[e] = std::stoi(index);
                }

                triangle[i] = {
                    positions[idx[0] - 1],
                    texcoords[idx[1] - 1],
                    normals[idx[2] - 1]
                };
            }
            // 頂点の順序を逆順にして左手系対応
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        } else if (identifier == "mtllib") {
            std::string mtlFile;
            s >> mtlFile;
            modelData.material = LoadMaterialTemplateFile(directoryPath, mtlFile);
        }
    }

    return modelData;
}
