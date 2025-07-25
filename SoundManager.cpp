#include "SoundManager.h"

void SoundManager::Initialize()
{

    //==XAudio�G���W���̃C���X�^���X�𐶐�==//
    HRESULT result_ = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(result_));
    //==�}�X�^�[�{�C�X�𐶐�==//
    result_ = xAudio2->CreateMasteringVoice(&masterVoice);
    assert(SUCCEEDED(result_));
}


void SoundManager::Finalize()
{
    xAudio2.Reset();
    SoundUnload(&soundData1);
}




// �`�����N�w�b�_
struct ChunkHeader {
    char id[4]; // �`�����NID
    uint32_t size; // �`�����N�T�C�Y
};
// RIFF�w�b�_�`�����N
struct RiffHeader {
    ChunkHeader chunk; // �`�����N�w�b�_(RIFF)
    char type[4]; // �t�H�[�}�b�g�i"WAVE"�j
};
// FMT�`�����N
struct FormatChunk {
    ChunkHeader chunk; // �`�����N�w�b�_(FMT)
    WAVEFORMATEX fmt; // WAVE�t�H�[�}�b�g
};


SoundData SoundManager::SoundLoadWave(const char* filename)
{
    HRESULT result;

    std::ifstream file(filename, std::ios::binary);
    assert(file.is_open());

    // RIFF�w�b�_�[�̓ǂݍ���
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0 || strncmp(riff.type, "WAVE", 4) != 0) {
        assert(0);
    }

    FormatChunk format = {};
    ChunkHeader chunk = {};

    // �`�����N�����ɓǂݎ���� fmt �� data ��T��
    char* pBuffer = nullptr;
    unsigned int dataSize = 0;

    while (file.read((char*)&chunk, sizeof(chunk))) {
        if (strncmp(chunk.id, "fmt ", 4) == 0) {
            assert(chunk.size <= sizeof(WAVEFORMATEX));
            file.read((char*)&format.fmt, chunk.size);
        } else if (strncmp(chunk.id, "data", 4) == 0) {
            pBuffer = new char[chunk.size];
            file.read(pBuffer, chunk.size);
            dataSize = chunk.size;
        } else {
            // ���̃`�����N�iJUNK�Ȃǁj�̓X�L�b�v
            file.seekg(chunk.size, std::ios::cur);
        }

        // fmt �� data ���ǂݍ��߂���I���
        if (format.fmt.nChannels != 0 && pBuffer != nullptr) {
            break;
        }
    }

    file.close();

    assert(format.fmt.nChannels != 0); // fmt �`�����N��������Ȃ�����
    assert(pBuffer != nullptr); // data �`�����N��������Ȃ�����

    SoundData soundData = {};
    soundData.wfex = format.fmt;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
    soundData.bufferSize = dataSize;
    return soundData;
}

//==�����f�[�^���==//
void SoundManager::SoundUnload(SoundData* soundData)
{

    // �o�b�t�@�̃����������
    delete[] soundData->pBuffer;

    soundData->pBuffer = 0;
    soundData->bufferSize = 0;
    soundData->wfex = {};
}

void SoundManager::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{
    HRESULT result;

    // �g�`�t�H�[�}�b�g�����Ƃ�sourceVoice�̐���
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
    assert(SUCCEEDED(result));

    // �Đ�����g�`�f�[�^�̐ݒ�
    XAUDIO2_BUFFER buf {};
    buf.pAudioData = soundData.pBuffer;
    buf.AudioBytes = soundData.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;

    // �g�`�f�[�^�̍Đ�
    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start();
}