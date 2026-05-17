#include <DxLib.h>
#include "../../Manager/SceneManager.h"
#include "AnimationController.h"

AnimationController::AnimationController(int modelId)
{
	modelId_ = modelId;
}
AnimationController::~AnimationController(void)
{
}

// アニメーション追加の共通処理
void AnimationController::Add(int type, float speed, const std::string path)
{
	Animation animation;
	animation.model = MV1LoadModel(path.c_str());
	animation.animIndex = -1;

	Add(type, speed, animation);
}

// 同じFBX内のアニメーションを準備
void AnimationController::AddInFbx(int type, float speed, int animIndex)
{
	Animation animation;
	animation.model = -1;
	animation.animIndex = animIndex;
	animation.speed = speed;

	Add(type, speed, animation);


	if (animations_.count(type) == 0)
	{
		// 動的配列に追加
		animations_.emplace(type, animation);
	}
}

void AnimationController::Play(int type, bool isLoop)
{
	if (playType_ == type)
	{
		// 同じアニメーションだったら再生を継続する
		return;
	}

	// アニメーションが再生中じゃなかったら
	if (playType_ != -1)
	{
		// モデルからアニメーションを外す
		MV1DetachAnim(modelId_, playAnim_.attachNo);
		playAnim_.attachNo = -1;
	}

	// アニメーション種別を変更
	playType_ = type;
	playAnim_ = animations_[type];

	// ループフラグ
	isLoop_ = isLoop;

	// モデルと同じFBX内か別かでアタッチ方法を分ける
  	if (playAnim_.model == -1)
	{
		// モデルと同じファイルからアニメーションをアタッチする
		playAnim_.attachNo = MV1AttachAnim(modelId_, playAnim_.animIndex);
	}
	else
	{
		// 別のモデルファイルからアニメーションをアタッチする
		// DxModelViewerを確認すること(大体0か1)
		int animIndex = 0;
		playAnim_.attachNo = MV1AttachAnim(modelId_, animIndex, playAnim_.model);
	}

	// アニメーション総時間の取得
	playAnim_.totalTime = MV1GetAnimTotalTime(modelId_, playAnim_.animIndex);

	// アニメーション時間初期化
	playAnim_.step = 0.0f;
}

void AnimationController::Update(void)
{
	// 経過時間の取得
	float deltaTime = SceneManager::GetInstance().GetDeltaTime();
	// 再生
	playAnim_.step += (deltaTime * playAnim_.speed);

	// アニメーションループ
	// ループフラグが立っていたら
	if (isLoop_)
	{
		// アニメーションの時間が総時間を超えたら
		if (playAnim_.step >= playAnim_.totalTime)
		{
			// アニメーションの時間を0に戻す
			playAnim_.step = 0.0f;
		}
	}

	// アニメーション設定
	MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);
}

void AnimationController::Draw(void)
{
	// アニメーションのデバッグ描画
	DrawFormatString(0, 20, 0xffffff, "AnimationType:%d", playType_);

	DrawFormatString(0, 60, 0xffffff, "attachNo:%d", playAnim_.attachNo);
	DrawFormatString(0, 80, 0xffffff, "step:%.2f / %.2f", playAnim_.step, playAnim_.totalTime);


	DrawFormatString(0, 100, 0xffffff, "speed:%.2f", playAnim_.speed);
	int animNum = MV1GetAnimNum(modelId_);
	DrawFormatString(0, 120, 0xffffff, "AnimNum: %d", animNum);

	if (playAnim_.attachNo == -1) {
		DrawFormatString(0, 140, 0xffffff, "AttachAnim failed");
	}
	else {
		DrawFormatString(0, 140, 0xffffff, "AttachAnim success");
	}

	DrawFormatString(0, 200, 0xffffff, "ExternalAnimNum: %d", MV1GetAnimNum(playAnim_.model));

}

void AnimationController::Release(void)
{
	//// 外部FBXのモデル(アニメーション)解放
	//for (const std::pair<int, Animation>& pair : animations_)
	//{
	//	if (pair.second.model != -1)
	//	{
	//		MV1DeleteModel(pair.second.model);
	//	}
	//}

	// ↓変数の型推論(auto)

	// 外部FBXのモデル(アニメーション)解放
	for (const auto& pair : animations_)
	{
		if (pair.second.model != -1)
		{
			MV1DeleteModel(pair.second.model);
		}
	}

	// 可変長配列をクリアする
	animations_.clear();
}

bool AnimationController::IsEnd(void) const
{
	bool ret = false;

	// 再生中なら
	if (isLoop_)
	{
		// ループ設定されていたら
		// 無条件で終了してない（flase）を返す
		return false;
	}

	// アニメーションが終了したら
	if (playAnim_.step >= playAnim_.totalTime)
	{
		// 再生終了
		return true;
	}

	return ret;
}

int AnimationController::GetPlayType(void) const
{
	return playType_;
}

void AnimationController::Add(int type, float speed, Animation& animation)
{
	// アニメーションのスピードを設定
	animation.speed = speed;
	
	if (animations_.count(type) == 0)
	{
		// 動的配列に追加
		animations_.emplace(type, animation);
	}
}

