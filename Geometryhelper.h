#pragma once
#define _USE_MATH_DEFINES
#include "CommonStructs.h"
      // M_PI �� sinf/cosf �p
// ���̒��_�𐶐�����֐�
void GenerateSphereVertices(
    VertexData* vertices,
    int kSubdivision,
    float radius);
