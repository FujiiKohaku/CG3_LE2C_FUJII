#include "Wave.h"
float totalHeight(Vector3 pos, float time)
{
    float height = 0.0f;

    // 中心からの距離
    float r = sqrtf(pos.x * pos.x + pos.z * pos.z);

    // 基本波（低周波の円形波）
    float freq1 = 5.0f; // 波の密度（1秒に5回波が通過するイメージ）
    float amp1 = 0.15f; // 振幅
    float speed1 = 2.0f; // 波の進む速さ
    height += sinf(freq1 * r - speed1 * time) * amp1;

    // 細かい波紋（高周波）
    float freq2 = 20.0f;
    float amp2 = 0.05f;
    float speed2 = 3.5f;
    height += sinf(freq2 * r - speed2 * time) * amp2;

    // 少し違う波形を加えてリズムを多様化
    float freq3 = 10.0f;
    float amp3 = 0.1f;
    float speed3 = 1.2f;
    height += sinf(freq3 * r * 0.5f - speed3 * time * 0.7f) * amp3;

    return height;
}
void GenerateGridVertices(VertexData* vertices, int kSubdivision,
    float gridSize, float time)
{
  
    const float cellSize = gridSize / kSubdivision; // 1マスの幅

    const float half = gridSize / 2.0f; //	中心を原点に合わせるための補正値

    for (int row = 0; row < kSubdivision; ++row) {
        float z = -half + cellSize * row; // 球と一緒///今ここ
        float nextZ = z + cellSize; // Z方向に一マスずつ増やしてる//nextZは次何メートル進みますよ―的な

        for (int col = 0; col < kSubdivision; ++col) {
            float x = -half + cellSize * col;
            float nextX = x + cellSize; // X方向に一マスずつ増やしてる
            // nextXとか名前変かもしれん
            // 手前と奥だからんかいい感じのを考えないといけないね。

            // 各頂点の位置
            Vector3 leftTop = { x, 0.0f, z }; // 左上
            Vector3 leftBottom = { x, 0.0f, nextZ }; // 左下
            Vector3 rightTop = { nextX, 0.0f, z }; // 右上
            Vector3 rightBottom = { nextX, 0.0f, nextZ }; // 右下

            // 高さ変化をサイン波で
            float freq = 2.0f; // 周波数
            float amp = 0.2f; // 振幅//波もう少し調整ノイズ作成
            leftTop.y = totalHeight(leftTop, time);
            leftBottom.y = totalHeight(leftBottom, time);
            rightTop.y = totalHeight(rightTop, time);
            rightBottom.y = totalHeight(rightBottom, time);

            // 頂点データ作成
            VertexData vA = {
                { leftTop.x, leftTop.y, leftTop.z, 1.0f },
                { (leftTop.x + half) / gridSize, 1.0f - (leftTop.z + half) / gridSize },
                { 0, 1, 0 }
            };
            VertexData vB = { { leftBottom.x, leftBottom.y, leftBottom.z, 1.0f },
                { (leftBottom.x + half) / gridSize,
                    1.0f - (leftBottom.z + half) / gridSize },
                { 0, 1, 0 } };
            VertexData vC = { { rightTop.x, rightTop.y, rightTop.z, 1.0f },
                { (rightTop.x + half) / gridSize,
                    1.0f - (rightTop.z + half) / gridSize },
                { 0, 1, 0 } };
            VertexData vD = { { rightBottom.x, rightBottom.y, rightBottom.z, 1.0f },
                { (rightBottom.x + half) / gridSize,
                    1.0f - (rightBottom.z + half) / gridSize },
                { 0, 1, 0 } };

            // 頂点インデックス
            int index = (row * kSubdivision + col) * 6;

            // 三角形1: A-B-C
            vertices[index + 0] = vA;
            vertices[index + 1] = vB;
            vertices[index + 2] = vC;

            // 三角形2: C-B-D
            vertices[index + 3] = vC;
            vertices[index + 4] = vB;
            vertices[index + 5] = vD;
        }
    }
}
void GenerateFloorVertices(VertexData* vertices, float size)
{
    const float half = size * 0.5f;

    // 4 頂点（左手座標系: +Y が上）
    Vector3 p0 = { -half, 0.0f, -half }; // 左上
    Vector3 p1 = { half, 0.0f, -half }; // 右上
    Vector3 p2 = { -half, 0.0f, half }; // 左下
    Vector3 p3 = { half, 0.0f, half }; // 右下

   
    vertices[0].position = { p0.x, p0.y, p0.z, 1.0f };
    vertices[1].position = { p2.x, p2.y, p2.z, 1.0f };
    vertices[2].position = { p1.x, p1.y, p1.z, 1.0f };

 
    vertices[3].position = { p1.x, p1.y, p1.z, 1.0f };
    vertices[4].position = { p2.x, p2.y, p2.z, 1.0f };
    vertices[5].position = { p3.x, p3.y, p3.z, 1.0f };

    // 共有設定（テクスチャ座標と法線）
    for (int i = 0; i < 6; ++i) {
  
        vertices[i].texcoord = { (vertices[i].position.x + half) / size,
            1.0f - (vertices[i].position.z + half) / size };
        
        vertices[i].normal = { 0.0f, 1.0f, 0.0f };
    }
}
void GenerateFlatGridVertices(VertexData* vertices, int kSubdivision, float gridSize)
{
    const float cellSize = gridSize / kSubdivision;
    const float half = gridSize / 2.0f;

    for (int row = 0; row < kSubdivision; ++row) {
        float z = -half + cellSize * row;
        float nextZ = z + cellSize;

        for (int col = 0; col < kSubdivision; ++col) {
            float x = -half + cellSize * col;
            float nextX = x + cellSize;

            // y = 0 の平面
            Vector3 leftTop = { x, 0.0f, z };
            Vector3 leftBottom = { x, 0.0f, nextZ };
            Vector3 rightTop = { nextX, 0.0f, z };
            Vector3 rightBottom = { nextX, 0.0f, nextZ };

            VertexData vA = {
                { leftTop.x, leftTop.y, leftTop.z, 1.0f },
                { (leftTop.x + half) / gridSize, 1.0f - (leftTop.z + half) / gridSize },
                { 0, 1, 0 }
            };
            VertexData vB = {
                { leftBottom.x, leftBottom.y, leftBottom.z, 1.0f },
                { (leftBottom.x + half) / gridSize, 1.0f - (leftBottom.z + half) / gridSize },
                { 0, 1, 0 }
            };
            VertexData vC = {
                { rightTop.x, rightTop.y, rightTop.z, 1.0f },
                { (rightTop.x + half) / gridSize, 1.0f - (rightTop.z + half) / gridSize },
                { 0, 1, 0 }
            };
            VertexData vD = {
                { rightBottom.x, rightBottom.y, rightBottom.z, 1.0f },
                { (rightBottom.x + half) / gridSize, 1.0f - (rightBottom.z + half) / gridSize },
                { 0, 1, 0 }
            };

            int index = (row * kSubdivision + col) * 6;
            vertices[index + 0] = vA;
            vertices[index + 1] = vB;
            vertices[index + 2] = vC;
            vertices[index + 3] = vC;
            vertices[index + 4] = vB;
            vertices[index + 5] = vD;
        }
    }
}
