#include "../Manager/ResourceManager.h"
<<<<<<< HEAD
=======
#include "../Manager/InputManager.h"
>>>>>>> c43593c588c266bd4e7c5e84d0a77fe0702bb34e
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
