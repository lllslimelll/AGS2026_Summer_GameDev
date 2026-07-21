#include "CollisionProfile.h"

CollisionProfile::CollisionProfile(CollisionProfileType type, CollisionChannel channel)
    : type_(type)
    , channel_(channel)
{
    responses_.fill(CollisionResponse::NONE);
}

void CollisionProfile::SetResponse(CollisionChannel channel, CollisionResponse response)
{
	responses_[static_cast<int>(channel)] = response;
}

CollisionResponse CollisionProfile::GetResponse(CollisionChannel channel) const
{
    // NONE ÇÕîÕàÕäOÇ»ÇÃÇ≈ÉKÅ[Éh
    if (channel == CollisionChannel::NONE) return CollisionResponse::NONE;

    int index = static_cast<int>(channel);
    if (index < 0 || index >= static_cast<int>(CollisionChannel::MAX))
        return CollisionResponse::NONE;

    return responses_[index];
}
