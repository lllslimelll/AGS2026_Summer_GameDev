// Collision/CollisionQueryParams.h
#pragma once
#include <vector>
class ActorBase;

struct CollisionQueryParams
{
    // –³Ž‹‚·‚é Actor ƒŠƒXƒg
    std::vector<const ActorBase*> ignoredActors;
};