#include "CollisionProfile.h"

CollisionProfile::CollisionProfile(CollisionProfileType type, CollisionChannel channel)
	: 
	type_(type),
	channel_(channel)
{
	// ‘Sƒ`ƒƒƒ“ƒlƒ‹”»’è”½‰ž–³Ž‹‚É‰Šú‰»
	responses_[CollisionChannel::WORLD_STATIC] = CollisionResponse::IGNORE;
	responses_[CollisionChannel::WORLD_DYNAMIC] = CollisionResponse::IGNORE;
	responses_[CollisionChannel::PAWN] = CollisionResponse::IGNORE;
	responses_[CollisionChannel::PHYSICS_BODY] = CollisionResponse::IGNORE;
	responses_[CollisionChannel::CAMERA] = CollisionResponse::IGNORE;
	responses_[CollisionChannel::TRIGGER] = CollisionResponse::IGNORE;
	responses_[CollisionChannel::VISIBILITY] = CollisionResponse::IGNORE;
}

void CollisionProfile::SetResponse(CollisionChannel channel, CollisionResponse response)
{
	responses_[channel] = response;
}

CollisionResponse CollisionProfile::GetResponse(CollisionChannel channel) const
{
	auto it = responses_.find(channel);
	if (it != responses_.end())
	{
		return it->second;
	}

	return CollisionResponse::IGNORE;
}
