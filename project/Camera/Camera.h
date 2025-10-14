#pragma once
#include "MatrixMath.h"
#include "Struct.h"
#include "WinApp.h"
struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};

class Camera {
public:
    // �f�t�H���g�R���X�g���N�^�錾
    Camera();

    // �X�V
    void Update();
    // ===============================
    // setter�i�O������l��ݒ�j
    // ===============================
    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
    void SetFovY(float fovY) { fovY_ = fovY; }
    void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
    void SetNearClip(float nearClip) { nearClip_ = nearClip; }
    void SetFarClip(float farClip) { farClip_ = farClip; }

    // ===============================
    // getter�i�O������l���擾�j
    // ===============================

    // �e�s��
    const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
    const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
    const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
    // ViewProjection�i�����ςݍs��j
    const Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
    // �e��Transform���
    const Vector3& GetRotate() const { return transform_.rotate; }
    const Vector3& GetTranslate() const { return transform_.translate; }

    // �e��v���W�F�N�V�����ݒ�l
    float GetFovY() const { return fovY_; }
    float GetAspectRatio() const { return aspectRatio_; }
    float GetNearClip() const { return nearClip_; }
    float GetFarClip() const { return farClip_; }

private:
    Transform transform_;
    Matrix4x4 worldMatrix_;
    Matrix4x4 viewMatrix_;
    Matrix4x4 projectionMatrix_;
    Matrix4x4 viewProjectionMatrix_;
    // �v���W�F�N�V�����v�Z�p�p�����[�^
    float fovY_ = 0.45f; // ���������̎���p
    float aspectRatio_ = static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight);
    float nearClip_ = 0.1f; // �j�A�N���b�v����
    float farClip_ = 100.0f; // �t�@�[�N���b�v����
};
