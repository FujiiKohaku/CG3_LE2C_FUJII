#include "object3d.hlsli"

cbuffer Material : register(b0)
{
    float4 color;
    int enableLighting;
    int lightingMode;
    float2 pad;
    matrix uvTransform;
};

cbuffer Light : register(b1)
{
    float4 lightColor;
    float3 lightDir;
    float intensity;
};

struct PSInput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 L = normalize(-lightDir); // lightDir ÇÕ VSÅ®PS Ç≈ worldãÛä‘ÇÃëOíÒ

    float lighting = 1.0f;

    if (enableLighting != 0)
    {
        float NdotL = dot(N, L);

        if (lightingMode == 1)
        {
            // Lambert
            lighting = saturate(NdotL);
        }
        else if (lightingMode == 2)
        {
            // Half-Lambert
            lighting = pow(NdotL * 0.5f + 0.5f, 2.0f);
        }
        // mode == 0 ÇÃèÍçá lighting = 1.0
    }

    float3 litColor = color.rgb * lightColor.rgb * lighting * intensity;
    return float4(litColor, color.a);
}
