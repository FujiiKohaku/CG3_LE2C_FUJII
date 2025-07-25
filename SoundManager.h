#pragma once
#include <cassert>
#include <fstream>
#include <string>
#include <wrl.h>
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")
// --------------------------------------
// XAudio2 ���g�p�����T�E���h�Ǘ��N���X
// �EWAV�t�@�C���̓ǂݍ���
// �E�Đ�����
// �E���������
// �E�������ƏI������
// --------------------------------------

// WAV�t�@�C���̓��e���i�[����\����
struct SoundData {
    WAVEFORMATEX wfex; // �����t�H�[�}�b�g���
    BYTE* pBuffer; // �����f�[�^�o�b�t�@
    unsigned int bufferSize; // �o�b�t�@�T�C�Y�i�o�C�g�P�ʁj
};

// �T�E���h�S�̂��Ǘ�����N���X
class SoundManager {
    // �R���X�g���N�^�E�f�X�g���N�^
    SoundManager();
    ~SoundManager();

    // XAudio2 ������������
    void Initialize() { }
    // XAudio2 ���������
    void Finalize();
    // WAV�t�@�C����ǂݍ���Ń������ɓW�J
    SoundData SoundLoadWave(const char* filename);
    // �����f�[�^��������������
    void SoundUnload(SoundData* soundData);
    // �������Đ�
    void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

private:
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
    IXAudio2MasteringVoice* masterVoice;

    SoundData soundData1;
};
