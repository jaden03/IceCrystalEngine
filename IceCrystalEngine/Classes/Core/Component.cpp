#include <Ice/Core/Component.h>
#include <iostream>

Component::Component()
{
	owner = nullptr;
	transform = nullptr;
}

void Component::Initialize()
{
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

void Component::OverlayUpdate()
{
}