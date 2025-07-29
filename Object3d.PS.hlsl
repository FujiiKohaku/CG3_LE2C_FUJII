#include "object3d.hlsli"

// 定数バッファ
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

// テクスチャとサンプラ
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// 出力構造体
struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // UVをアフィン変換する
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    float lighting = 1.0f;

    if (gMaterial.enableLighting != 0)
    {
        float3 normal = normalize(input.normal);
        float3 lightDir = normalize(-gDirectionalLight.direction);

        if (gMaterial.lightingMode == 1)
        {
            // Lambert
            lighting = saturate(dot(normal, lightDir));
        }
        else if (gMaterial.lightingMode == 2)
        {
            // Half-Lambert
            lighting = pow(dot(normal, lightDir) * 0.5f + 0.5f, 2.0f);
        }
        // gMaterial.lightingMode == 0 のとき lighting = 1.0（つまりライティングなし）
    }

    output.color = gMaterial.color * textureColor * lighting;

    return output;
}
