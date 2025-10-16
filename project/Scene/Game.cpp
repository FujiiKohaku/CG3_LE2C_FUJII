#include "Game.h"
#include "TitleScene.h"

void Game::Initialize()
{

	winApp_ = new WinApp();
	winApp_->initialize();

	dxCommon_ = new DirectXCommon();
	dxCommon_->Initialize(winApp_);

	input_ = new Input();
	input_->Initialize(winApp_);

	sceneManager_ = new SceneManager();
	sceneManager_->SetSystem(winApp_, dxCommon_);
	BaseScene* scene = new TitleScene();
	sceneManager_->SetNextScene(scene);

}

void Game::Update()
{
	input_->Update();

	sceneManager_->Update(input_);
}

void Game::Draw()
{

	sceneManager_->Draw();
}

void Game::Finalize()
{
	delete sceneManager_;
	delete dxCommon_;
	if (winApp_) { winApp_->Finalize(); delete winApp_; }
	delete input_;
}
