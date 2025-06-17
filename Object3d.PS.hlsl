#include "object3d.hlsli"


ConstantBuffer<Material> gMaterial : register(b0);


ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;

    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);

   
    
    if (gMaterial.enableLigting != 0)//Lighting‚·‚éê‡
    {
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;

    }
    else
    { //Lighting‚µ‚È‚¢ê‡‘O‰ñ‚Ü‚Å‚Æ“¯‚¶ŒvZ
        output.color = gMaterial.color * textureColor;
    }
    
    
    return output;
}
