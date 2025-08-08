#pragma once
#include "DebugCamera.h"
#include "MatrixMath.h"
#include "CommonStructs.h"
class WVPManager {
public:
    // 毎フレーム行列を更新
    void Update(const Transform& transform, const DebugCamera& camera, float width, float height);
    void Update(const Transform& transform, float width, float height,float nearZ = 0.0f, float farZ = 100.0f); // ビュー=単位, 正射影

    // Getter
    const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
    const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
    const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
    const Matrix4x4& GetWVPMatrix() const { return worldViewProjectionMatrix_; }

private:
    Matrix4x4 worldMatrix_;
    Matrix4x4 viewMatrix_;
    Matrix4x4 projectionMatrix_;
    Matrix4x4 worldViewProjectionMatrix_;
};
