// WVPBuffer.cpp
#include "WVPBuffer.h"

void WVPBuffer::Create(ID3D12Device* device)
{
    // バッファ作成
    wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));

    // アドレス取得
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));

    // 初期値：単位行列を代入
    Matrix4x4 identity = MatrixMath::MakeIdentity4x4();
    wvpData->WVP = identity;
    wvpData->World = identity;
}

void WVPBuffer::Update(const Matrix4x4& wvp, const Matrix4x4& world)
{
    if (wvpData) {
        wvpData->WVP = wvp;
        wvpData->World = world;
    }
}
