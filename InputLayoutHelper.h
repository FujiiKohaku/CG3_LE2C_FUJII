#pragma once
#include <d3d12.h>

class InputLayoutHelper {
public:
    // 標準の POSITION + TEXCOORD + NORMAL レイアウトを返す
    static D3D12_INPUT_LAYOUT_DESC CreatePosTexNormLayout();
};
