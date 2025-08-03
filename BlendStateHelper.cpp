#include "BlendStateHelper.h"

D3D12_BLEND_DESC BlendStateHelper::CreateWriteAll()
{
    D3D12_BLEND_DESC desc {};
    desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    return desc;
}
