#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "SceneManager.h"
#include "SceneBase.h"

SceneBase::SceneBase(void)
	: 
	resMng_(ResourceManager::GetInstance()),
	sceMng_(SceneManager::GetInstance()),
	inputMng_(InputManager::GetInstance())
{
}

SceneBase::~SceneBase(void)
{
}

void SceneBase::Init(void)
{
}

void SceneBase::Update(void)
{
}

void SceneBase::Draw(void)
{
}
