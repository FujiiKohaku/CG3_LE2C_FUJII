#pragma once
#include <d3d12.h>

class BlendStateHelper {
public:
    // 全ての色要素を書き込むブレンド設定
    static D3D12_BLEND_DESC CreateWriteAll();
};
