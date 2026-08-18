#pragma once
#include <string>
#include <map>

class AnimationController
{
public:

    // アニメーションデータ
    struct Animation
    {
        int   model = -1;
        int   attachNo = -1;
        int   animIndex = 0;
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
    void Play(int type, bool isLoop = true);
    void Replay(void) { playAnim_.step = 0.0f; }

    // 更新
    void Update(void);

    // 解放
    void Release(void);

    // 再生中のアニメーション種別
    int GetPlayType(void) const;

    // 再生終了
    bool IsEnd(void) const;

    // 再生中のアニメーション情報を取得
    const Animation& GetPlayAnim(void) const;

    // ブレンド中か
    bool IsBlending(void) const;

    // ブレンド時間（秒）の設定
    void SetBlendTime(float blendTime) { blendTime_ = blendTime; }

private:

    // アニメーションするモデルのハンドルID
    int modelId_;

    // 種類別のアニメーションデータ
    std::map<int, Animation> animations_;

    // 再生中のアニメーション
    int       playType_;
    Animation playAnim_;
    bool      isLoop_;

    // ブレンド用（前のアニメーション）
    Animation prevAnim_;
    int       prevType_;
    float     blendRate_;   // 0.0=prev, 1.0=current
    float     blendTime_;   // ブレンドにかける時間（秒）

    // アニメーション追加の共通処理
    void Add(int type, float speed, Animation& animation);

    // アタッチ処理
    void Attach(Animation& anim);
};