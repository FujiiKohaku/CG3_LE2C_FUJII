#include "object3d.hlsli"

//���߂Ă�VertexShader/CG2_02_00
//struct TransformationMatrix
//{
//    float32_t4x4 WVP;�@//������hlsl�ɏ������Ă��������炻�����ɏ�����
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

