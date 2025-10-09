#include "Object3d.h"
#include "Object3dManager.h"
#include <fstream>
#include <sstream> // �� ���ꂪ�K�v�I
void Object3d::Initialize(Object3dManager* object3DManager)
{
    // �����Ŏ󂯎���ă����o�ϐ��ɋL�^
    object3dManager_ = object3DManager;

    // ���f���ǂݍ���
    modelData = LoadObjFile("resources", "plane.obj");

    // ���_���\�[�X�����
    vertexResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size()); // ���_�f�[�^�����\�[�X�ɃR�s�[
}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
    // 1.���ŕK�v�ƂȂ�ϐ��̐錾
    MaterialData materialData; // �\�z����MaterialData
    // 2.�t�@�C�����J��
    std::string line; // �t�@�C������ǂ񂾂P�s���i�[�������
    std::ifstream file(directoryPath + "/" + filename); // �t�@�C�����J��
    assert(file.is_open()); // �Ƃ肠�����J���Ȃ�������~�߂�
    // 3.���ۂɃt�@�C����ǂ݁AMaterialData���\�z���Ă���
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;
        // identifier�ɉ���������
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // �A�����ăt�@�C���p�X�ɂ���
            materialData.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }
    // 4.materialData��Ԃ�
    return materialData;
}

Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string filename)
{
    // 1.���ŕK�v�ƂȂ�ϐ��̐錾
    ModelData modelData; // �\�z����ModelData
    std::vector<Vector4> positions; // �ʒu
    std::vector<Vector3> normals; // �@��
    std::vector<Vector2> texcoords; // �e�N�X�`�����W
    std::string line; // �t�@�C������ǂ񂾈�s���i�[�������

    // 2.�t�@�C�����J��
    std::ifstream file(directoryPath + "/" + filename); // �t�@�C�����J��
    assert(file.is_open()); // �Ƃ肠�����J���Ȃ�������~�߂�

    // 3.���ۂɃt�@�C����ǂ�,ModelData���\�z���Ă���
    while (std::getline(file, line)) {
        std::string identifiler;
        std::istringstream s(line);
        s >> identifiler; // �擪�̎��ʎq��ǂ�

        // identifier�ɉ���������
        if (identifiler == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            // ������W�ɂ���
            position.x *= -1.0f;

            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifiler == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            // �㉺�t�ɂ���

            // texcoord.y *= -1.0f;
            texcoord.y = 1.0f - texcoord.y;
            // CG2_06_02_kusokusosjsusuawihoafwhgiuwhkgfau
            texcoords.push_back(texcoord);
        } else if (identifiler == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            // ������W�ɂ���
            normal.x *= -1.0f;

            normals.push_back(normal);
        } else if (identifiler == "f") {
            VertexData triangle[3]; // �O�̒��_��ۑ�
            // �ʂ͎O�p�`����B���̑��͖��Ή�
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // ���_�̗v�f�ւ�Index�́u�ʒu/UV/�@���v�Ŋi�[����Ă���̂ŁA�������Ă�Index���擾����
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;

                    std::getline(v, index, '/'); // ��؂�ŃC���f�b�N�X��ǂ�ł���
                    elementIndices[element] = std::stoi(index);
                }
                // �v�f�ւ�Index����A���ۂ̗v�f�̒l���擾���āA���_���\�z����
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];
                // X���𔽓]���č�����W�n��

                triangle[faceVertex] = { position, texcoord, normal };
            }
            // �t���ɂ��Ċi�[�i2 �� 1 �� 0�j
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
            //?
        } else if (identifiler == "mtllib") {
            // materialTemplateLibrary�t�@�C���̖��O���擾����
            std::string materialFilename;
            s >> materialFilename;
            // ��{�I��obj�t�@�C���Ɠ���K�wmtl�͑��݂�����̂ŁA�f�B���N�g�����ƃt�@�C������n���B
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    // 4.ModelData��Ԃ�
    return modelData;
}
