#pragma once
#define _USE_MATH_DEFINES
#include "CommonStructs.h"
      // M_PI や sinf/cosf 用
// 球の頂点を生成する関数
void GenerateSphereVertices(
    VertexData* vertices,
    int kSubdivision,
    float radius);
