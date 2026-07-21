// GameScene.h
#pragma once
#include "SceneBase.h"
#include <vector>
#include <memory>
class ActorBase;
class Player;
class Camera;
class StageManager;
class ItemManager;
class EnemyManager;
class SkyDome;
class GameUI;

class GameScene : public SceneBase
{
public:

    GameScene(void);
    ~GameScene(void) override;

    void Load(void) override;
    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

    // Actor を生成して管理リストに追加する
    template<typename T, typename... Args>
    T* SpawnActor(Args&&... args)
    {
        auto actor = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = actor.get();
        actors_.push_back(std::move(actor));
        return ptr;
    }

    // Actor を取得する
    template<typename T>
    T* GetActor(void) const
    {
        for (auto& actor : actors_)
        {
            if (auto* ptr = dynamic_cast<T*>(actor.get()))
            {
                return ptr;
            }
        }
        return nullptr;
    }

private:

    // 全 Actor を一元管理
    std::vector<std::unique_ptr<ActorBase>> actors_;

    // UI（Actor ではないので別管理）
    std::vector<std::unique_ptr<GameUI>> gameUIs_;

    // 今は直接参照が必要なものだけポインタで持つ
    Player* player_ = nullptr;
    Camera* camera_ = nullptr;
    std::unique_ptr<StageManager>  stageMng_;
    std::unique_ptr<ItemManager>   itemMng_;
    std::unique_ptr<EnemyManager>  enemyMng_;
    SkyDome* skyDome_ = nullptr;

    int CreateShadowMap(void);
};