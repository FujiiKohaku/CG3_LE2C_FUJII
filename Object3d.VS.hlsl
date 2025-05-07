//‰‚ß‚Ä‚ÌVertexShader/CG2_02_00
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
    output.position = input.position;
    return output;
}