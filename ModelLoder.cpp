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

ModelDataNoUV LoadObjFileNoTexture(const std::filesystem::path& directoryPath, const std::string& filename)
{
    ModelDataNoUV modelData;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::string line;

    // ファイルを開く
    std::filesystem::path filePath = directoryPath / filename;
    std::ifstream file(filePath.string());
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "v") {
            // 頂点座標
            Vector4 position {};
            s >> position.x >> position.y >> position.z;
            position.x *= -1.0f; // 右手系→左手系
            position.w = 1.0f;
            positions.push_back(position);

        } else if (identifier == "vn") {
            // 法線
            Vector3 normal {};
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1.0f; // 右手系→左手系
            normals.push_back(normal);

        } else if (identifier == "f") {
            // 面（頂点インデックス）
            VertexDataNoUV triangle[3];
            for (int i = 0; i < 3; ++i) {
                std::string vertexDefinition;
                s >> vertexDefinition;

                std::istringstream v(vertexDefinition);
                std::string posIndexStr, texDummy, normIndexStr;

                std::getline(v, posIndexStr, '/'); // v
                std::getline(v, texDummy, '/'); // vt（無視）
                std::getline(v, normIndexStr, '/'); // vn

                int posIndex = std::stoi(posIndexStr);
                int normIndex = std::stoi(normIndexStr);

                triangle[i].position = positions[posIndex - 1];
                triangle[i].normal = normals[normIndex - 1];
            }

            // 左手系なので逆順で追加
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        }
    }

    return modelData;
}
