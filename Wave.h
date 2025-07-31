#pragma once
#include "CommonStructs.h"
#include "MatrixMath.h"
struct Wave {
    Vector2 center;
    float startTime;
};





float totalHeight(Vector3 pos, float time);
void GenerateGridVertices(VertexData* vertices, int kSubdivision,float gridSize, float time);
void GenerateFloorVertices(VertexData* vertices, float size);
void GenerateFlatGridVertices(VertexData* vertices, int kSubdivision, float gridSize);