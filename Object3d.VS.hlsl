//‰‚ß‚Ä‚ÌVertexShader/CG2_02_00
struct TransformationMatrix
{
    float32_t4x4 WVP;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
    
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
};

struct VertexSgaderInput
{
    float32_t4 position : POSITION0;
};


VertexShaderOutput main(VertexSgaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);

    return output;
}

