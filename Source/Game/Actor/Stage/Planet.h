#pragma once
#include <vector>
#include <string>
#include "../ActorBase.h"

class Planet : public ActorBase
{
public:

    // è’ìÀîªíËéÌï 
    enum class COLLIDER_TYPE
    {
        MODEL = 0,
        MAX,
    };

    ~Planet(void) override;

    void Update(void) override;
    void Draw(void)   override;

protected:

    virtual void InitLoad(void)      override;
    virtual void InitTransform(void) override;
    virtual void InitCollider(void)  override;
    virtual void InitAnimation(void) override;
    virtual void InitPost(void)      override;

private:

    const std::vector<std::string> EXCLUDE_FRAME_NAMES = {};
    const std::vector<std::string> TARGET_FRAME_NAMES = { "Ground" };
};