#include "Geometryhelper.h"
#include <cmath> // for sinf, cosf

void GenerateSphereVertices(VertexData* vertices, int kSubdivision, float radius)
{
    const float kLonEvery = static_cast<float>(M_PI * 2.0f) / kSubdivision; // 経度刻み
    const float kLatEvery = static_cast<float>(M_PI) / kSubdivision; // 緯度刻み

    for (int latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        float lat = -static_cast<float>(M_PI) / 2.0f + kLatEvery * latIndex;

        for (int lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
            float lon = kLonEvery * lonIndex;

            // 頂点A
            Vector3 posA = {
                cosf(lat) * cosf(lon),
                sinf(lat),
                cosf(lat) * sinf(lon)
            };
            VertexData vertA = {
                { posA.x * radius, posA.y * radius, posA.z * radius, 1.0f },
                { static_cast<float>(lonIndex) / kSubdivision,
                    1.0f - static_cast<float>(latIndex) / kSubdivision },
                posA // 法線 = 単位球の位置ベクトル
            };

            // 頂点B
            Vector3 posB = {
                cosf(lat + kLatEvery) * cosf(lon),
                sinf(lat + kLatEvery),
                cosf(lat + kLatEvery) * sinf(lon)
            };
            VertexData vertB = {
                { posB.x * radius, posB.y * radius, posB.z * radius, 1.0f },
                { static_cast<float>(lonIndex) / kSubdivision,
                    1.0f - static_cast<float>(latIndex + 1) / kSubdivision },
                posB
            };

            // 頂点C
            Vector3 posC = {
                cosf(lat) * cosf(lon + kLonEvery),
                sinf(lat),
                cosf(lat) * sinf(lon + kLonEvery)
            };
            VertexData vertC = {
                { posC.x * radius, posC.y * radius, posC.z * radius, 1.0f },
                { static_cast<float>(lonIndex + 1) / kSubdivision,
                    1.0f - static_cast<float>(latIndex) / kSubdivision },
                posC
            };

            // 頂点D
            Vector3 posD = {
                cosf(lat + kLatEvery) * cosf(lon + kLonEvery),
                sinf(lat + kLatEvery),
                cosf(lat + kLatEvery) * sinf(lon + kLonEvery)
            };
            VertexData vertD = {
                { posD.x * radius, posD.y * radius, posD.z * radius, 1.0f },
                { static_cast<float>(lonIndex + 1) / kSubdivision,
                    1.0f - static_cast<float>(latIndex + 1) / kSubdivision },
                posD
            };

            // 書き込み
            uint32_t startIndex = (latIndex * kSubdivision + lonIndex) * 6;
            vertices[startIndex + 0] = vertA;
            vertices[startIndex + 1] = vertB;
            vertices[startIndex + 2] = vertC;

            vertices[startIndex + 3] = vertC;
            vertices[startIndex + 4] = vertB;
            vertices[startIndex + 5] = vertD;
        }
    }
}
