#pragma once
class SpriteManager;
class Sprite {
public:
    // ������
    void Initialize(SpriteManager* spriteManager);

private:
    // SpriteManager�̃|�C���^
    SpriteManager* spriteManager_ = nullptr;
    // ���_�f�[�^
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };
    // �o�b�t�@���\�[�X
    //   �o�b�t�@���\�[�X�iGPU�p�f�[�^�j
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource; // ���_�o�b�t�@
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource; // �C���f�b�N�X�o�b�t�@
                                                          // CPU���̃f�[�^�Q�Ɨp
    VertexData* vertexData = nullptr;
    uint32_t* indexData = nullptr;

    // �o�b�t�@�r���[�i�g�������j
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
};
