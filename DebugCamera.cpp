#include "DebugCamera.h"

void DebugCamera::Update()
{
    // カメラのワールド行列を更新
    cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotation_, translation_);
    viewMatrix = Inverse(cameraMatrix);


    

}

void DebugCamera::Initialize()
{
    // ワールド変換行列を作成（スケール1、回転・移動はメンバ変数から）
    cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotation_, translation_);

    // ビュー行列はカメラ行列の逆行列
    viewMatrix = Inverse(cameraMatrix);

    // 正射影行列の作成（左上・右下・近クリップ・遠クリップ）
    orthoGraphicMatrix = MakeOrthographicMatrix(-160.0f, 160.0f, 200.0f, 300.0f, 0.0f, 1000.0f);
}