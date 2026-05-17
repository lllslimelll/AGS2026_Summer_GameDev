#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../Collider/ColliderBase.h"
#include "../../Collider/ColliderModel.h"
#include "Item.h"

Item::Item()
    :
    ActorBase(),
    isAimed_(false)
{
}

Item::~Item()
{
}

void Item::Update(void)
{
}

void Item::Draw(void)
{


#ifdef _DEBUG
    // 照準が当たっているときスフィアを緑、通常は赤で表示
    int color = isAimed_ ? 0x00ff00 : 0xff0000;
    DrawSphere3D(transform_.pos, 30.0f, 8, color, color, FALSE);
#endif
}

void Item::OnPickedUp(void)
{
}

void Item::OnThrow(const VECTOR& throwDir)
{
}

void Item::OnHitEnemy(void)
{
}

int Item::GetValue(void) const
{
    return 0;
}

Item::GRADE Item::GetGrade(void) const
{
    return grade_;
}

Item::STATE Item::GetState(void) const
{
    return state_;
}

void Item::SetAimed(bool aimed)
{
    isAimed_ = aimed;
}

void Item::InitLoad(void)
{
}

void Item::InitTransform(void)
{
}

void Item::InitCollider(void)
{
}

void Item::InitAnimation(void)
{
}

void Item::InitPost(void)
{
}

void Item::ChangeState(STATE state)
{
}

void Item::ChangeStateDropped(void)
{
}

void Item::ChangeStateHeld(void)
{
}

void Item::ChangeStateFlying(void)
{
}

void Item::UpdateDropped(void)
{
}

void Item::UpdateHeld(void)
{
}

void Item::UpdateFlying(void)
{
}

void Item::UpdateThrowMove(void)
{
}

void Item::DrawBillboard(void) const
{
}
