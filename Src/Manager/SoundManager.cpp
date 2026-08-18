#include "SoundManager.h"
#include "InputManager.h"
#include "DxLib.h"

// グローバル領域で変数を初期化
SoundManager* SoundManager::instance_ = nullptr;

SoundManager::SoundManager(void)
{
}

void SoundManager::SetBgmVolume(int vol)
{
	ChangeVolumeSoundMem(vol, bgmTitleH_);
	ChangeVolumeSoundMem(vol, bgmGameH_);
}

void SoundManager::SetSeVolume(int vol)
{
	ChangeVolumeSoundMem(vol, walkH_);
	ChangeVolumeSoundMem(vol, pickUpH_);
	ChangeVolumeSoundMem(vol, boostH_);
	ChangeVolumeSoundMem(vol, damagedH_);
	ChangeVolumeSoundMem(vol, selectH_);
	ChangeVolumeSoundMem(vol, resultH_);
}

//インスタンスの生成
void SoundManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new SoundManager;
	}

	instance_->Init();
}

SoundManager& SoundManager::GetInstance(void)
{
	return *instance_;
}

void SoundManager::Init(void)
{
	bgmTitleH_ = LoadSoundMem(BGM_TITLE_PATH);
	bgmGameH_ = LoadSoundMem(BGM_GAME_PATH);
	walkH_ = LoadSoundMem(WALK_PATH);
	pickUpH_ = LoadSoundMem(PICKUP_PATH);
	boostH_ = LoadSoundMem(BOOST_PATH);
	damagedH_ = LoadSoundMem(DAMAGED_PATH);
	selectH_ = LoadSoundMem(SELECT_PATH);
	resultH_ = LoadSoundMem(RESULT_PATH);
	alertH_ = LoadSoundMem(ALERT_PATH);
	damaged2H_ = LoadSoundMem(DAMAGED2_PATH);
	foundH_ = LoadSoundMem(FOUND_PATH);

	// 音量調整
	ChangeVolumeSoundMem(BGM_TITLE_VOLUME, bgmTitleH_);
	ChangeVolumeSoundMem(BGM_GAME_VOLUME, bgmGameH_);
	ChangeVolumeSoundMem(WALK_VOLUME, walkH_);
	ChangeVolumeSoundMem(PICKUP_VOLUME, pickUpH_);
	ChangeVolumeSoundMem(BOOST_VOLUME, boostH_);
	ChangeVolumeSoundMem(DAMAGED_VOLUME, damagedH_);
	ChangeVolumeSoundMem(SELECT_VOLUME, selectH_);
	ChangeVolumeSoundMem(RESULT_VOLUME, resultH_);
	ChangeVolumeSoundMem(ALERT_VOLUME, alertH_);
	ChangeVolumeSoundMem(DAMAGED2_VOLUME, damaged2H_);
	ChangeVolumeSoundMem(FOUND_VOLUME, foundH_);

}

void SoundManager::Update(void)
{
}

void SoundManager::Destroy(void)
{
	DeleteSoundMem(bgmTitleH_);
	DeleteSoundMem(bgmGameH_);
	DeleteSoundMem(walkH_);
	DeleteSoundMem(pickUpH_);
	DeleteSoundMem(boostH_);
	DeleteSoundMem(damagedH_);
	DeleteSoundMem(selectH_);
	DeleteSoundMem(resultH_);
	DeleteSoundMem(alertH_);
	DeleteSoundMem(damaged2H_);
	DeleteSoundMem(foundH_);

	// インスタンスのメモリ解放
	delete instance_;
}

void SoundManager::PlayBgmTitle()
{
	// 再生
	if (CheckSoundMem(bgmTitleH_) == 0)
	{
		PlaySoundMem(bgmTitleH_, DX_PLAYTYPE_LOOP, true);
	}
}

void SoundManager::PlayBGMGame()
{
	// 再生
	if (CheckSoundMem(bgmGameH_) == 0)
	{
		PlaySoundMem(bgmGameH_, DX_PLAYTYPE_LOOP, true);
	}
}

void SoundManager::PlayWalk()
{
	// 再生
	if (CheckSoundMem(walkH_) == 0)
	{
		PlaySoundMem(walkH_, DX_PLAYTYPE_LOOP, true);
	}
}

void SoundManager::PlayPickUp()
{
	PlaySoundMem(pickUpH_, DX_PLAYTYPE_BACK, true);
}

void SoundManager::PlayBoost()
{
	PlaySoundMem(boostH_, DX_PLAYTYPE_BACK, true);
}

void SoundManager::PlayDamaged()
{
	PlaySoundMem(damagedH_, DX_PLAYTYPE_BACK, true);
}

void SoundManager::PlaySelect()
{
	PlaySoundMem(selectH_, DX_PLAYTYPE_BACK, true);
}

void SoundManager::PlayResult()
{
	PlaySoundMem(resultH_, DX_PLAYTYPE_BACK, true);
}

void SoundManager::PlayAlert()
{
	if (CheckSoundMem(alertH_) == 0)
	{
		PlaySoundMem(alertH_, DX_PLAYTYPE_LOOP, true);
	}
}

void SoundManager::PlayDamaged2()
{
	if (CheckSoundMem(damaged2H_) == 0)
	{
		PlaySoundMem(damaged2H_, DX_PLAYTYPE_BACK, true);
	}
}

void SoundManager::PlayFound()
{
	// 再生中なら重ねない（発見ボイスが多重に鳴らないように）
	if (CheckSoundMem(foundH_) == 0)
	{
		PlaySoundMem(foundH_, DX_PLAYTYPE_BACK, true);
	}
}

void SoundManager::StopBGMTitle()
{
	StopSoundMem(bgmTitleH_);
}

void SoundManager::StopBGMGame()
{
	StopSoundMem(bgmGameH_);
}

void SoundManager::StopWalk()
{
	StopSoundMem(walkH_);
}

void SoundManager::StopAlert()
{
	StopSoundMem(alertH_);
}

void SoundManager::StopPickUp()
{
	StopSoundMem(pickUpH_);
}

void SoundManager::StopDamaged()
{
	StopSoundMem(damagedH_);
}
