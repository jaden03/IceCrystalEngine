#pragma once

#ifndef ACTOR_H

#define ACTOR_H

class Transform; // forward declaration because actor and transform depend on eachother

class Actor
{

public:
	
	Transform* transform;

	Actor();
	~Actor();

};



#endif