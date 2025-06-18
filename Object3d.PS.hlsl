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

   
    
    if (gMaterial.enableLighting != 0)//LightingÇ∑ÇÈèÍçá
    {
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
        //half lambert
        //float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        //float cos =  pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        output.color = cos * gMaterial.color * textureColor;
        
    }
    else
    { //LightingÇµÇ»Ç¢èÍçáëOâÒÇ‹Ç≈Ç∆ìØÇ∂åvéZ
        output.color = gMaterial.color * textureColor;
    }
    
    
    return output;
}
