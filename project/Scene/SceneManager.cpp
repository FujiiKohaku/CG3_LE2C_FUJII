#include "SceneManager.h"

void SceneManager::Update(Input* input)
{

	// ���V�[���̗\�񂪂���Ȃ�
	if (nextScene_) {
		// ���V�[���̏I��
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		// �V�[���؂�ւ�
		scene_ = nextScene_;
		nextScene_ = nullptr;

		scene_->SetSceneManager(this);
		scene_->SetSystem(winApp_, dxCommon_);
		// ���V�[����������
		scene_->Initialize();
	}

	scene_->Update(input);
}

void SceneManager::Draw()
{
	scene_->Draw();
}

SceneManager::~SceneManager() {
	// �Ŋ��̃V�[���̏I���Ɖ��
	scene_->Finalize();
	delete scene_;
}
