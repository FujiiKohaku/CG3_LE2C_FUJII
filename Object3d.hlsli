//03_00
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};
struct VertexShaderInput//k
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};
struct Material//k
{
    float32_t4 color;
    int32_t enableLigting;
};
struct TransformationMatrix//k
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
struct DirectionalLight//k
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};