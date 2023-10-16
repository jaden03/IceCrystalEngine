#include <Ice/Core/Transform.h>

Transform::Transform(Actor* owner)
{
	actor = owner;
}

Transform::~Transform()
{
	delete actor;
}