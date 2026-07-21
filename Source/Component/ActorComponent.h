#pragma once
class ActorBase;

// アクターコンポーネント
// 全コンポーネントの基底クラス
class ActorComponent
{
public:

    ActorComponent(ActorBase& owner);
    virtual ~ActorComponent(void);

    virtual void Init(void) {}
    virtual void Update(void) {}
    // デバッグ描画
    virtual void Draw(void) {}
    virtual void Release(void) {}


    // 所有者を取得する
    ActorBase& GetOwner(void) const;

    // 有効フラグ
    bool IsActive(void) const;
    // 有効フラグを設定
    void SetActive(bool active);

private:

    // このコンポーネントを持つアクター
    ActorBase& owner_;
    // 有効フラグ
    bool active_ = true;
};