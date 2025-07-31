cbuffer Transform : register(b0)
{
    matrix worldViewProjection;
    matrix worldMatrix;
}
cbuffer Light : register(b1)
{
    float4 lightColor;
    float3 lightDir;
    float intensity;
}

struct VSInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.svpos = mul(input.position, worldViewProjection);
    output.normal = mul((float3x3) worldMatrix, input.normal);
    return output;
}
