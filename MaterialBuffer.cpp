#include "MaterialBuffer.h"

void MaterialBuffer::Create(ID3D12Device* device, const Material& material)
{
    materialResource_ = CreateBufferResource(device, sizeof(Material));
    Update(material);
}

void MaterialBuffer::Update(const Material& material)
{
    Material* mappedData = nullptr;
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
    *mappedData = material;
    materialResource_->Unmap(0, nullptr);
}
