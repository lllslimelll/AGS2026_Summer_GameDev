#pragma once
#include <memory>
#include <vector>
#include <chrono>
#include <DxLib.h>

class SceneBase;
class Fader;
class Camera;

class SceneManager
{

public:

	// 背景色
	static constexpr int BACKGROUND_COLOR_R = 0;
	static constexpr int BACKGROUND_COLOR_G = 0;
	static constexpr int BACKGROUND_COLOR_B = 0;

	// ディレクショナルライトの方向
	static constexpr VECTOR LIGHT_DIRECTION = { 0.3f, -0.7f, 0.8f };

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		GAME,
		PAUSE,
		DEAD,
		RESULT,
		DEBUG,
	};
	
	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	// 初期化
	void Init(void);
	
	// 3Dの初期化
	void Init3D(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

	// 状態遷移
	void ChangeScene(SCENE_ID nextId);
	// オーバーレイの追加（オーバーレイとして）
	void PushOverlay(SCENE_ID sceneId);
	// オーバーレイの削除
	void PopOverlay();

	// カメラの取得
	Camera& GetCamera(void);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const;
	void SetResultScore(int score);
	int  GetResultScore(void) const;

private:
	int resultScore_ = 0;
	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);

	// コピーコンストラクタも同様
	SceneManager(const SceneManager& instance) = default;

	// デストラクタも同様
	~SceneManager(void) = default;

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// 各種シーン
	std::vector<std::unique_ptr<SceneBase>> scenes_;

	// フェード
	std::unique_ptr<Fader> fader_;

	// カメラ
	std::unique_ptr<Camera> camera_;

	// メインスクリーン　
	int mainScreen_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// 前フレームの時間
	std::chrono::system_clock::time_point preTime_;
	// デルタタイム
	float deltaTime_;

	// シーン遷移実行
	void DoChangeScene(SCENE_ID sceneId);

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// フェード
	void Fade(void);

};