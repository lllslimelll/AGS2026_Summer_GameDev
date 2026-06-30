#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "SceneManager.h"
#include "Scene.h"

Scene::Scene(void)
	: 
	resMng_(ResourceManager::GetInstance()),
	sceMng_(SceneManager::GetInstance()),
	inputMng_(InputManager::GetInstance())
{
}

Scene::~Scene(void)
{
}

void Scene::Init(void)
{
}

void Scene::Update(void)
{
}

void Scene::Draw(void)
{
}
