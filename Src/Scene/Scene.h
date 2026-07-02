#pragma once
class ResourceManager;
class SceneManager;
class InputManager;

class Scene
{

public:

	// コンストラクタ
	Scene(void);

	// デストラクタ
	virtual ~Scene(void) = 0;

	// 初期化
	virtual void Init(void) = 0;

	// 更新
	virtual void Update(void) = 0;

	// 描画
	virtual void Draw(void) = 0;

protected:

	// リソース管理
	ResourceManager& resMng_;

	// シーン管理
	SceneManager& sceMng_;

	// 入力管理
	InputManager& inputMng_;

};
