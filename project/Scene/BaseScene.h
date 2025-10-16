#pragma once

class SceneManager;
class Input;
class WinApp;
class DirectXCommon;

class BaseScene {
public:
    virtual ~BaseScene() = default;
    virtual void Initialize() = 0;
    virtual void Update(Input* input) = 0;
    virtual void Draw() = 0;
    virtual void Finalize() = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
    virtual void SetSystem(WinApp* wa, DirectXCommon* dx) { winApp_ = wa; dxCommon_ = dx; }

protected:
	SceneManager* GetSceneManager() const { return sceneManager_; }
    WinApp* GetWinApp() const { return winApp_; }
    DirectXCommon* GetDx() const { return dxCommon_; }

private:
	SceneManager* sceneManager_ = nullptr;
    WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
};

