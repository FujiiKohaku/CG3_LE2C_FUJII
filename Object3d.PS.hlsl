#include "object3d.hlsli"

// �萔�o�b�t�@
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

// �e�N�X�`���ƃT���v��
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// �s�N�Z���V�F�[�_�[�o�͍\����
struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // --- UV�ϊ� ---
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    // --- ���C�e�B���O�̏����l ---
    float lighting = 1.0f;

    if (gMaterial.enableLighting != 0)
    {
        // ���͂��ꂽ�@���̓��[���h��ԁiVS�ŕϊ��ς݁j
        float3 normal = normalize(input.normal);

        // ���s�����̕����i���[���h��ԁj
        float3 lightDir = normalize(-gDirectionalLight.direction);

        if (gMaterial.lightingMode == 1)
        {
            // Lambert����
            lighting = saturate(dot(normal, lightDir));
        }
        else if (gMaterial.lightingMode == 2)
        {
            // Half-Lambert����
            lighting = pow(dot(normal, lightDir) * 0.5f + 0.5f, 2.0f);
        }
        // mode == 0 �̂Ƃ��� lighting = 1.0�i=���C�e�B���O�Ȃ��j
    }

    // --- �ŏI�F�̌v�Z ---
    output.color = gMaterial.color * textureColor * lighting;

    return output;
}
