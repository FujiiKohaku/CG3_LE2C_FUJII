#include "object3d.hlsli"

//‰‚ß‚Ä‚ÌVertexShader/CG2_02_00
//struct TransformationMatrix
//{
//    float32_t4x4 WVP;@//‘—¿‚Éhlsl‚É‘‚¯‚Á‚Ä‚ ‚Á‚½‚©‚ç‚»‚Á‚¿‚É‘‚­‚æ
//};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
    


struct VertexSgaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};


VertexShaderOutput main(VertexSgaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    return output;
}

