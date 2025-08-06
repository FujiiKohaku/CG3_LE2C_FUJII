#include "WVPManager.h"

void WVPManager::Update(const Transform& transform, const DebugCamera& camera, float width, float height)
{
    worldMatrix_ = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    viewMatrix_ = camera.GetViewMatrix();
    float aspect = width / height;
    projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(0.45f, aspect, 0.1f, 100.0f);
    worldViewProjectionMatrix_ = MatrixMath::Multiply(worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
}
