#pragma once
#include "ActorBase.h"

class StaticMeshComponent;

class SkyDome : public ActorBase
{

public:

	// ó‘Ô
	enum class STATE
	{
		NONE,
		STAY,
		FOLLOW
	};

	SkyDome(const ActorBase* followTransform);

	~SkyDome() override;

	void Init(void)   override;
	// XV
	void Update(void) override;

	// •`‰æ
	void Draw(void) override;

private:

	// ’Ç]‘ÎÛ‚ÌTransform
	const ActorBase* followTarget_;

	StaticMeshComponent* mesh_ = nullptr;

	// ó‘Ô
	STATE state_;

	// ó‘Ô‘JˆÚ
	void ChangeState(STATE state);
	// XV
	void UpdateStay(void);
	void UpdateFollow(void);
};