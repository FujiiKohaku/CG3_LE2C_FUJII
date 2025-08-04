#pragma once
#include <d3d12.h>

class ViewportManager {
public:
    ViewportManager(int width, int height);

    const D3D12_VIEWPORT& GetViewport() const { return viewport_; }
    const D3D12_RECT& GetScissorRect() const { return scissorRect_; }

private:
    D3D12_VIEWPORT viewport_;
    D3D12_RECT scissorRect_;
};
