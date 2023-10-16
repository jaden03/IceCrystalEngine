#include <Ice/Core/Actor.h>
#include <Ice/Core/Transform.h> // include transform here for the full definition

Actor::Actor()
{
	transform = new Transform(this);
}

Actor::~Actor()
{
	delete transform;
}