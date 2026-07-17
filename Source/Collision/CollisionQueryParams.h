#pragma once
#include <vector>

class ActorBase;

struct CollisionQueryParams
{
	// –³Ž‹‚·‚é Actor ƒŠƒXƒg
	std::vector<const ActorBase*> ignoredActors_;

	// –³Ž‹‚·‚é Actor ‚ð’Ç‰Á
	void AddIgnoredActor(const Actorbase* actor)
	{
		
	}
};