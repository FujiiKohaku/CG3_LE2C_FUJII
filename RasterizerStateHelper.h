#pragma once
#include <d3d12.h>

class RasterizerStateHelper {
public:
    // 裏面カリング + 塗りつぶし
    static D3D12_RASTERIZER_DESC CreateDefault();
};
