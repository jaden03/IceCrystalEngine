#include <Ice/Core/Component.h>

Component::Component(Actor* owner)
{
	actor = owner;
}

Component::~Component()
{
	delete actor;
}