#include "TitleScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "GamePlayScene.h"

void TitleScene::Initialize() {
}

void TitleScene::Update(Input* input) {
	if (input->IsKeyPressed(DIK_RETURN)) {
		GetSceneManager()->SetNextScene(new GamePlayScene());
	}
}

void TitleScene::Draw() {
	GetDx()->PreDraw();

	GetDx()->PostDraw();
}

void TitleScene::Finalize() {
}
