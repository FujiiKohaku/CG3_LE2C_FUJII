#include "RasterizerStateHelper.h"

D3D12_RASTERIZER_DESC RasterizerStateHelper::CreateDefault()
{
    // RasiterzerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc {};
    // 裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    return rasterizerDesc;
}
