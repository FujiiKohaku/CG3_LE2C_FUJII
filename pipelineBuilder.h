#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>

class PipelineBuilder {
public:
    void SetRootSignature(ID3D12RootSignature* rootSignature);
    void SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layout);
    void SetVertexShader(IDxcBlob* vs);
    void SetPixelShader(IDxcBlob* ps);
    void SetBlendState(const D3D12_BLEND_DESC& blend);
    void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizer);
    void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencil);
    void SetRTVFormat(DXGI_FORMAT format);
    void SetDSVFormat(DXGI_FORMAT format);
    void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

    Microsoft::WRL::ComPtr<ID3D12PipelineState> Build(ID3D12Device* device);

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_ = {};
};
