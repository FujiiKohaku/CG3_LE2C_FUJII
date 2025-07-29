//03_00
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

//05_03
struct Material 
{
    float4 color;
    int enableLighting;
    int lightingMode; // 0: None, 1: Lambert, 2: Half-Lambert
    float2 padding; // 16バイトアラインメント
    matrix uvTransform;
};
//05_03
struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};