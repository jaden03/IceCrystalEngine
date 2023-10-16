#include <Ice/Core/Component.h>

Component::Component()
{
	owner = nullptr;
}

Component::~Component()
{
	delete owner;
}

void Component::Update()
{
}