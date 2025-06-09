#include "object3d.hlsli"

//���߂Ă�VertexShader/CG2_02_00
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);


VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix.World));
    return output;
}

