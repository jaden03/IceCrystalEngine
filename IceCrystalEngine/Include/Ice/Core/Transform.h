#pragma once

#ifndef TRANSFORM_H

#define TRANSFORM_H

#include <Ice/Core/Actor.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform
{

public:

	Actor* actor;
	Transform* parent = nullptr;
	std::vector<Transform*>* children = new std::vector<Transform*>();

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 eulerAngles;
	glm::vec3 scale;
	
	glm::vec3 localPosition;
	glm::quat localRotation;
	glm::vec3 localEulerAngles;
	glm::vec3 localScale;


	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	Transform(Actor* owner);

	// Update (dont use)
	void Update();

	void SetParent(Transform* parent);

	void LookAt(float x, float y, float z);
	void LookAt(glm::vec3 target);
	
	// Helper Functions
	
	// Add To Position
	void Translate(glm::vec3 translation);
	// Add To Position
	void Translate(float x, float y, float z);
	// Add To Position (Scaled By Delta Time)
	void TranslateDelta(glm::vec3 translation);
	// Add To Position (Scaled By Delta Time)
	void TranslateDelta(float x, float y, float z);
	
	// Add To Rotation
	void Rotate(glm::vec3 rotation);
	// Add To Rotation
	void Rotate(float x, float y, float z);
	// Add To Rotation (Scaled By Delta Time)
	void RotateDelta(glm::vec3 rotation);
	// Add To Rotation (Scaled By Delta Time)
	void RotateDelta(float x, float y, float z);

	// If you actually find a use for these, your weird
	// Add To Scale
	void Scale(glm::vec3 scale);
	// Add To Scale
	void Scale(float x, float y, float z);
	// Add To Scale (Scaled By Delta Time)
	void ScaleDelta(glm::vec3 scale);
	// Add To Scale (Scaled By Delta Time)
	void ScaleDelta(float x, float y, float z);
	// --------------------------------------------- \\

	// Set Position Directly
	void SetPosition(glm::vec3 position);
	// Set Position Directly
	void SetPosition(float x, float y, float z);

	// Set Rotation Directly
	void SetRotation(glm::vec3 rotation);
	// Set Rotation Directly
	void SetRotation(float x, float y, float z);
	// Set Rotation Directly
	void SetRotation(glm::quat rotation);

	// Set Scale Directly
	void SetScale(glm::vec3 scale);
	// Set Scale Directly
	void SetScale(float x, float y, float z);
	



	// Helper Functions for Local Transformations

	// Add To Local Position
	void TranslateLocal(glm::vec3 translation);
	// Add To Local Position
	void TranslateLocal(float x, float y, float z);
	// Add To Local Position (Scaled By Delta Time)
	void TranslateLocalDelta(glm::vec3 translation);
	// Add To Local Position (Scaled By Delta Time)
	void TranslateLocalDelta(float x, float y, float z);

	// Add To Local Rotation
	void RotateLocal(glm::vec3 rotation);
	// Add To Local Rotation
	void RotateLocal(float x, float y, float z);
	// Add To Local Rotation (Scaled By Delta Time)
	void RotateLocalDelta(glm::vec3 rotation);
	// Add To Local Rotation (Scaled By Delta Time)
	void RotateLocalDelta(float x, float y, float z);

	// If you actually find a use for these, you're weird
	// Add To Local Scale
	void ScaleLocal(glm::vec3 scale);
	// Add To Local Scale
	void ScaleLocal(float x, float y, float z);
	// Add To Local Scale (Scaled By Delta Time)
	void ScaleLocalDelta(glm::vec3 scale);
	// Add To Local Scale (Scaled By Delta Time)
	void ScaleLocalDelta(float x, float y, float z);
	// --------------------------------------------- \\
	
	// Set Local Position Directly
	void SetLocalPosition(glm::vec3 position);
	// Set Local Position Directly
	void SetLocalPosition(float x, float y, float z);

	// Set Local Rotation Directly
	void SetLocalRotation(glm::vec3 rotation);
	// Set Local Rotation Directly
	void SetLocalRotation(float x, float y, float z);
	// Set Local Rotation Directly
	void SetLocalRotation(glm::quat rotation);

	// Set Local Scale Directly
	void SetLocalScale(glm::vec3 scale);
	// Set Local Scale Directly
	void SetLocalScale(float x, float y, float z);

private:

	glm::vec3 Forward();
	glm::vec3 Right();
	glm::vec3 Up();

};


#endif