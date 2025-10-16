#pragma once
#include "BaseScene.h"

class WinApp;
class DirectXCommon;

class SceneManager {
public:
	void Update(Input* input);
	void Draw();
	~SceneManager();

	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }
	void SetSystem(WinApp* wa, DirectXCommon* dx) { winApp_ = wa; dxCommon_ = dx; }

private:
	// ���̃V�[��
	BaseScene* scene_ = nullptr;
	// ���̃V�[��
	BaseScene* nextScene_ = nullptr;

	// �؂蕨
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
};

