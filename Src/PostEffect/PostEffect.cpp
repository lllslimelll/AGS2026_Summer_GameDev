#include <DxLib.h>
#include "../Manager/ScreenManager.h"
#include "../Common/Vector2.h"
#include "../Application.h"
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"
#include "PostEffect.h"

PostEffect::PostEffect(void)
	:
	targetScreen_(-1),
	enabled_(false),		// 無効
	pingPongScreens_()
{
}

PostEffect::~PostEffect(void)
{
}

void PostEffect::Init(int targetScreen)
{
	// 対象スクリーンを保存
	targetScreen_ = targetScreen;

	// 画面サイズのスクリーンを取得(ピンポンバッファ用)
	pingPongScreens_[0] =
		ScreenManager::GetInstance().GetPingPongScreen(0);
	pingPongScreens_[1] = ScreenManager::GetInstance().GetPingPongScreen(1);

	// エフェクトの初期化
	InitEffect();
}

void PostEffect::Draw(void)
{
	// 無効なら何もしない
	if (!enabled_) return;

	// 最初の読み込み元(オリジナル画面)
	int currentRead = targetScreen_;

	// 最初の書き込み先(ピンポンバッファ0番目)
	int currentWrite = pingPongScreens_[0];

	// 各エフェクトを順番に描画
	for (int i = 0; i < (int)materials_.size(); i++)
	{
		// 描画先を書き込み側に設定
		SetDrawScreen(currentWrite);
		ClearDrawScreen();

		// 入力テクスチャとして読み込み側をマテリアルに設定
		materials_[i]->SetTexture(0, currentRead);

		// レンダラーでエフェクトを描画
		renderers_[i]->Draw();

		// 読み込みと書き込みを入れ替える(ピンポン)
		currentRead = currentWrite;

		// 書き込み側は、使っていないもう片方のバッファに切り替える
		if (currentWrite == pingPongScreens_[0])
		{
			currentWrite = pingPongScreens_[1];
		}
		else
		{
			currentWrite = pingPongScreens_[0];
		}
	}

	// 描画先を対象スクリーンに設定
	SetDrawScreen(targetScreen_);
	// 最終的なエフェクトのスクリーンを描画
	DrawGraph(0, 0, currentRead, true);
}

// エフェクト適用設定
void PostEffect::SetEnabled(bool enabled)
{
	enabled_ = enabled;
}

// エフェクト追加
void PostEffect::Add(std::string shaderFileName, int constBufFloat4Size, int texSlotNum, int texAddress)
{
	// マテリアルの生成
	auto material = std::make_unique<PixelMaterial>(
		shaderFileName, constBufFloat4Size, texSlotNum, texAddress);

	// レンダラーの生成
	auto renderer = std::make_unique<PixelRenderer>(*material);

	// 描画矩形のサイズと座標(ポストエフェクトは画面サイズにだけかける想定)
	Vector2 size = { Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y };
	Vector2 pos = { 0, 0 };

	// 描画矩形の生成
	renderer->MakeSquareVertex(size, pos);

	// リストに追加
	materials_.push_back(std::move(material)); // マテリアル
	renderers_.push_back(std::move(renderer)); // レンダラー
}

// 定数バッファの値をセット
void PostEffect::SetConstBuffer(int effectIndex, int bufIndex, const FLOAT4& value)
{
	materials_[effectIndex]->SetConstBuffer(bufIndex, value);
}
