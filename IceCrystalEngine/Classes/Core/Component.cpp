#include <Ice/Core/Component.h>
#include <iostream>

Component::Component()
{
	owner = nullptr;
	transform = nullptr;
}

void Component::Ready()
{
}

void Component::Update()
{
}

void Component::LateUpdate()
{
}