#pragma once
#include <cassert>
#include <fstream>
#include <string>
#include <wrl.h>
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")
class SoundStruct {
public:
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
    IXAudio2MasteringVoice* masterVoice;
    void Initialize()
    {

        //==XAudio�G���W���̃C���X�^���X�𐶐�==//
        HRESULT result_ = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
        assert(SUCCEEDED(result_));
        //==�}�X�^�[�{�C�X�𐶐�==//
        result_ = xAudio2->CreateMasteringVoice(&masterVoice);
        assert(SUCCEEDED(result_));
    }

    void UpDate() { }

private:
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

    // �����f�[�^
    struct SoundData {
        // �g�`�t�H�[�}�b�g
        WAVEFORMATEX wfex;
        // �o�b�t�@�̐擪�A�h���X
        BYTE* pBuffer;
        // �o�b�t�@�̃T�C�Y
        unsigned int bufferSize;
    };

    SoundData SoundLoadWave(const char* filename);
};
