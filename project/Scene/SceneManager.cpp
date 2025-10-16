#include "SceneManager.h"

void SceneManager::Update(Input* input)
{

	// 次シーンの予約があるなら
	if (nextScene_) {
		// 旧シーンの終了
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		// シーン切り替え
		scene_ = nextScene_;
		nextScene_ = nullptr;

		scene_->SetSceneManager(this);
		scene_->SetSystem(winApp_, dxCommon_);
		// 次シーンを初期化
		scene_->Initialize();
	}

	scene_->Update(input);
}

void SceneManager::Draw()
{
	scene_->Draw();
}

SceneManager::~SceneManager() {
	// 最期のシーンの終了と解放
	scene_->Finalize();
	delete scene_;
}
