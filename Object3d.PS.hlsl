#include "object3d.hlsli"

// 定数バッファ
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

// テクスチャとサンプラ
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// ピクセルシェーダー出力構造体
struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // --- UV変換 ---
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    // --- ライティングの初期値 ---
    float lighting = 1.0f;

    if (gMaterial.enableLighting != 0)
    {
        // 入力された法線はワールド空間（VSで変換済み）
        float3 normal = normalize(input.normal);

        // 平行光源の方向（ワールド空間）
        float3 lightDir = normalize(-gDirectionalLight.direction);

        if (gMaterial.lightingMode == 1)
        {
            // Lambert反射
            lighting = saturate(dot(normal, lightDir));
        }
        else if (gMaterial.lightingMode == 2)
        {
            // Half-Lambert反射
            lighting = pow(dot(normal, lightDir) * 0.5f + 0.5f, 2.0f);
        }
        // mode == 0 のときは lighting = 1.0（=ライティングなし）
    }

    // --- 最終色の計算 ---
    output.color = gMaterial.color * textureColor * lighting;

    return output;
}
