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

        //==XAudioエンジンのインスタンスを生成==//
        HRESULT result_ = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
        assert(SUCCEEDED(result_));
        //==マスターボイスを生成==//
        result_ = xAudio2->CreateMasteringVoice(&masterVoice);
        assert(SUCCEEDED(result_));
    }

    void UpDate() { }

private:
    // チャンクヘッダ
    struct ChunkHeader {
        char id[4]; // チャンクID
        uint32_t size; // チャンクサイズ
    };
    // RIFFヘッダチャンク
    struct RiffHeader {
        ChunkHeader chunk; // チャンクヘッダ(RIFF)
        char type[4]; // フォーマット（"WAVE"）
    };
    // FMTチャンク
    struct FormatChunk {
        ChunkHeader chunk; // チャンクヘッダ(FMT)
        WAVEFORMATEX fmt; // WAVEフォーマット
    };

    // 音声データ
    struct SoundData {
        // 波形フォーマット
        WAVEFORMATEX wfex;
        // バッファの先頭アドレス
        BYTE* pBuffer;
        // バッファのサイズ
        unsigned int bufferSize;
    };

    SoundData SoundLoadWave(const char* filename);
};
