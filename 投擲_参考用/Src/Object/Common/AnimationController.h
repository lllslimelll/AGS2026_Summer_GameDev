#pragma once
#include <string>
#include <map>
class AnimationController
{
public:

	// アニメーションデータ
	struct Animation
	{
		int model = -1;
		int attachNo = -1;
		int animIndex = 0;
		float speed = 0.0f;
		float totalTime = 0.0f;
		float step = 0.0f;
	};
	// コンストラクタ
	AnimationController(int modelId);
	// デストラクタ
	~AnimationController(void);

	// 外部FBXからアニメーション追加
	void Add(int type, float speed, const std::string path);

	// 同じFBX内のアニメーションを準備
	void AddInFbx(int type, float speed, int animIndex);

	// アニメーション再生
	void Play(int type, bool isLoop);

	void Update(void);
	void Draw(void);
	void Release(void);

	bool IsEnd(void) const;	// 再生終了
	
	int GetPlayType(void) const;	// 再生中のアニメーション

private:

	// アニメーション追加の共通処理
	void Add(int type, float speed, Animation& animation);

	// アニメーションするモデルのハンドルID
	int modelId_;
	// ループフラグ
	bool isLoop_;

	// 種類別のアニメーションデータ
	std::map<int, Animation> animations_;

	// 再生中のアニメーション
	int playType_;
	Animation playAnim_;
};


