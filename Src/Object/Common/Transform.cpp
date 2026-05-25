#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
#include "Transform.h"

Transform::Transform(void)
	:
	modelId(-1),
	scl(AsoUtility::VECTOR_ONE),
	rot(AsoUtility::VECTOR_ZERO),
	pos(AsoUtility::VECTOR_ZERO),
	localPos(AsoUtility::VECTOR_ZERO),
	localOffset_(AsoUtility::VECTOR_ZERO),
	matScl(MGetIdent()),
	matRot(MGetIdent()),
	matPos(MGetIdent()),
	quaRot(Quaternion()),
	quaRotLocal(Quaternion()),
	parent_(nullptr)
{
}

Transform::~Transform(void)
{
}

void Transform::Update(void)
{
	if (parent_ != nullptr)
	{
		// ç¿ïWí«è]
		VECTOR rotatedOffset = parent_->quaRot.PosAxis(localOffset_);
		pos = VAdd(parent_->pos, rotatedOffset);

		// âÒì]í«è]
		quaRot = parent_->quaRot;
	}

	// ëÂÇ´Ç≥
	matScl = MGetScale(scl);

	// âÒì]
	rot = quaRot.ToEuler();
	matRot = quaRot.ToMatrix();

	// à íu
	matPos = MGetTranslate(pos);

	// çsóÒÇÃçáê¨
	MATRIX mat = MGetIdent();
	mat = MMult(mat, matScl);
	Quaternion q = quaRot.Mult(quaRotLocal);
	mat = MMult(mat, q.ToMatrix());
	mat = MMult(mat, matPos);

	// çsóÒÇÉÇÉfÉãÇ…îªíË
	if (modelId != -1)
	{
		MV1SetMatrix(modelId, mat);
	}

}

void Transform::Release(void)
{
}

void Transform::SetModel(int model)
{
	modelId = model;
}

VECTOR Transform::GetForward(void) const
{
	return GetDir(AsoUtility::DIR_F);
}

VECTOR Transform::GetBack(void) const
{
	return GetDir(AsoUtility::DIR_B);
}

VECTOR Transform::GetRight(void) const
{
	return GetDir(AsoUtility::DIR_R);
}

VECTOR Transform::GetLeft(void) const
{
	return GetDir(AsoUtility::DIR_L);
}

VECTOR Transform::GetUp(void) const
{
	return GetDir(AsoUtility::DIR_U);
}

VECTOR Transform::GetDown(void) const
{
	return GetDir(AsoUtility::DIR_D);
}

VECTOR Transform::GetDir(const VECTOR& dir) const
{
	return quaRot.PosAxis(dir);
}

void Transform::Attach(Transform* parent, VECTOR offset)
{
	parent_ = parent;
	localOffset_ = offset;
}

void Transform::Detach(void)
{
	parent_ = nullptr;
	localOffset_ = AsoUtility::VECTOR_ZERO;
}
