#include "SoundManager.h"
#include "InputManager.h"
#include "DxLib.h"

// グローバル領域で変数を初期化
SoundManager* SoundManager::instance_ = nullptr;

SoundManager::SoundManager(void)
{
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

	// 音量調整
	ChangeVolumeSoundMem(BGM_TITLE_VOLUME, bgmTitleH_);
	ChangeVolumeSoundMem(BGM_GAME_VOLUME, bgmGameH_);
	ChangeVolumeSoundMem(WALK_VOLUME, walkH_);
	ChangeVolumeSoundMem(PICKUP_VOLUME, pickUpH_);
	ChangeVolumeSoundMem(BOOST_VOLUME, boostH_);
	ChangeVolumeSoundMem(DAMAGED_VOLUME, damagedH_);

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

void SoundManager::PlayBgmGame()
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
	// 再生
	if (CheckSoundMem(boostH_) == 0)
	{
		PlaySoundMem(boostH_, DX_PLAYTYPE_LOOP, true);
	}
}

void SoundManager::PlayDamaged()
{
	PlaySoundMem(damagedH_, DX_PLAYTYPE_BACK, true);
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

void SoundManager::StopPickUp()
{
	StopSoundMem(pickUpH_);
}

void SoundManager::StopBoost()
{
	StopSoundMem(boostH_);
}

void SoundManager::StopDamaged()
{
	StopSoundMem(damagedH_);
}
