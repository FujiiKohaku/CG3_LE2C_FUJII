#pragma once

#include "CommonStructs.h"
#include "MatrixMath.h"

class UVTransformManager {
public:
    void Update(const Transform& transform); // 引数で受け取るように変更
    const Matrix4x4& GetMatrix() const;

private:
    Matrix4x4 uvMatrix_;
};
