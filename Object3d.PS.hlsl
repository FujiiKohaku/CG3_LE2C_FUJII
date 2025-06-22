#include "object3d.hlsli"

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

//  追加：時間を渡す定数バッファ（b2）
cbuffer TimeBuffer : register(b2)
{
    float time; // 秒単位で更新
}

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // ───────────────────────────────────────
    //  ベース色（テクスチャ × マテリアルカラー）
    // ───────────────────────────────────────
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f),
                               gMaterial.uvTransform);

    float4 texColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 baseColor = gMaterial.color.rgb * texColor.rgb;

    // ───────────────────────────────────────
    //  シンプルな半ランバートライティング（任意）
    // ───────────────────────────────────────
    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal),
                          -gDirectionalLight.direction);
        float lit = pow(saturate(NdotL) * 0.5f + 0.5f, 2.0f);
        baseColor *= lit * gDirectionalLight.color.rgb *
                      gDirectionalLight.intensity;
    }

    // ───────────────────────────────────────
    //  コースティクス模様（時間＋UVで生成）
    // ───────────────────────────────────────
    float2 uv = transformedUV.xy * 10.0f; // 細かさ調整
    float wave =
        sin(uv.x * 6.0f + time * 3.0f) +
        cos(uv.y * 12.0f - time * 4.0f);

    float caustics = pow(wave * 0.25f + 0.5f, 4.0f); // 0-1 に正規化→強調
    float3 causticsColor = caustics * float3(1.2f, 1.3f, 1.5f); // 白〜青み

    // ───────────────────────────────────────
    //  合成して出力
    // ───────────────────────────────────────
    float3 finalColor = saturate(baseColor + causticsColor);
    output.color = float4(finalColor, 1.0f);
    return output;
}
