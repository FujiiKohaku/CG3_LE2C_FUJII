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
float2 uv = transformedUV.xy * 15.0f;

// 複数方向から波を合成
float wave =
    sin(uv.x * 6.0f + time * 2.5f) +
    sin(uv.y * 8.0f + time * 3.0f) +
    sin((uv.x + uv.y) * 5.0f + time * 5.0f) +
    sin((uv.x - uv.y) * 7.0f - time * 3.5f);

// 正規化＆強調
float caustics = pow(wave * 0.125f + 0.5f, 4.0f);
float3 causticsColor = caustics * float3(0.4f, 1.3f, 1.5f);
    // ───────────────────────────────────────
    //  合成して出力
    // ───────────────────────────────────────
    float3 finalColor = saturate(baseColor + causticsColor);
    output.color = float4(finalColor, 1.0f);
    return output;
}
