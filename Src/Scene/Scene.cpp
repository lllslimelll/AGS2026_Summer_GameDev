#include "../Manager/ResourceManager.h"
#include "SceneManager.h"
#include "Scene.h"

Scene::Scene(void)
	: 
	resMng_(ResourceManager::GetInstance()),
	sceMng_(SceneManager::GetInstance())
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
