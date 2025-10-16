#pragma once
#include "SceneManager.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"

class Game {
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	WinApp* GetWinApp() const { return winApp_; }

private:
	// ÉVÅ[Éì
	SceneManager* sceneManager_ = nullptr;
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;

};