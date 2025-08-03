#include "pipelineBuilder.h"
#include <cassert>

void PipelineBuilder::SetRootSignature(ID3D12RootSignature* rootSignature)
{
    desc_.pRootSignature = rootSignature;
}

void PipelineBuilder::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layout)
{
    desc_.InputLayout = layout;
}

void PipelineBuilder::SetVertexShader(IDxcBlob* vs)
{
    desc_.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
}

void PipelineBuilder::SetPixelShader(IDxcBlob* ps)
{
    desc_.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
}

void PipelineBuilder::SetBlendState(const D3D12_BLEND_DESC& blend)
{
    desc_.BlendState = blend;
}

void PipelineBuilder::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizer)
{
    desc_.RasterizerState = rasterizer;
}

void PipelineBuilder::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencil)
{
    desc_.DepthStencilState = depthStencil;
}

void PipelineBuilder::SetRTVFormat(DXGI_FORMAT format)
{
    desc_.RTVFormats[0] = format;
    desc_.NumRenderTargets = 1;
}

void PipelineBuilder::SetDSVFormat(DXGI_FORMAT format)
{
    desc_.DSVFormat = format;
}

void PipelineBuilder::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
    desc_.PrimitiveTopologyType = type;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineBuilder::Build(ID3D12Device* device)
{
    desc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    desc_.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    HRESULT hr = device->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));
    return pipelineState;
}
