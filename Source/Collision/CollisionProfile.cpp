#include "CollisionProfile.h"

CollisionProfile::CollisionProfile(CollisionProfileType type, CollisionChannel channel)
	: 
	type_(type),
	channel_(channel)
{
	// ‘Sƒ`ƒƒƒ“ƒlƒ‹”»’è”½‰–³‹‚É‰Šú‰»
	responses_.fill(CollisionResponse::IGNORE);
}

void CollisionProfile::SetResponse(CollisionChannel channel, CollisionResponse response)
{
	responses_[static_cast<int>(channel)] = response;
}

CollisionResponse CollisionProfile::GetResponse(CollisionChannel channel) const
{
	return responses_[static_cast<int>(channel)];
}
