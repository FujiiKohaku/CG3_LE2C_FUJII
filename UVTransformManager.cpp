#include "UVTransformManager.h"

void UVTransformManager::Update(const Transform& transform)
{
    uvMatrix_ = MatrixMath::Matrix4x4MakeScaleMatrix(transform.scale);
    uvMatrix_ = MatrixMath::Multiply(uvMatrix_, MatrixMath::MakeRotateZMatrix(transform.rotate.z));
    uvMatrix_ = MatrixMath::Multiply(uvMatrix_, MatrixMath::MakeTranslateMatrix(transform.translate));
}

const Matrix4x4& UVTransformManager::GetMatrix() const
{
    return uvMatrix_;
}
