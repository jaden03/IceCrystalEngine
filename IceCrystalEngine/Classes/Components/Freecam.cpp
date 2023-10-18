#include <Ice/Components/Freecam.h>

#include <iostream>

#include <GLFW/glfw3.h>

Freecam::Freecam() : Component()
{
	speed = 5.0f;
	sensitivity = 0.1f;

	pitch = 0.0f;
	yaw = 0.0f;
	roll = 0.0f;
	
	input.GetMousePosition(&lastMouseX, &lastMouseY);
}


void Freecam::Ready()
{
	// Create the horizontal and vertical input axes (this will be moved to a better place later)
	// but for now you will just have to create them yourself in the code
	input.CreateAxis("Horizontal", GLFW_KEY_D, GLFW_KEY_A);
	input.CreateAxis("Vertical", GLFW_KEY_W, GLFW_KEY_S);
	
	input.lockCursor = true;
}

void Freecam::Update()
{
	float horizontal = input.GetAxis("Horizontal");
	float vertical = input.GetAxis("Vertical");

	// Move the camera
	glm::vec3 movement = (vertical * transform->forward) + (horizontal * transform->right);

	// Up and down
	if (input.GetKey(GLFW_KEY_SPACE))
		movement += glm::vec3(0, 1, 0);
	if (input.GetKey(GLFW_KEY_LEFT_CONTROL))
		movement -= glm::vec3(0, 1, 0);

	float currentSpeed = input.GetKey(GLFW_KEY_LEFT_SHIFT) ? speed * 1.5f : speed;
	currentSpeed = input.GetKey(GLFW_KEY_LEFT_ALT) ? speed * 0.2f : currentSpeed;

	if (glm::length(movement) != 0.0f)
		movement = glm::normalize(movement) * currentSpeed;
	
	transform->TranslateDelta(movement);

	// Rotate the camera
	double mouseX, mouseY;
	input.GetMousePosition(&mouseX, &mouseY);

	float deltaX = mouseX - lastMouseX;
	float deltaY = mouseY - lastMouseY;

	yaw += deltaX * sensitivity;

	pitch -= deltaY * sensitivity;
	pitch = glm::clamp(pitch, -89.0f, 89.0f);


	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw + 90)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw + 90)) * cos(glm::radians(pitch));
	direction = glm::normalize(direction);

	glm::mat4 transformView = glm::mat4(1.0f);
	transformView = glm::lookAt(transform->position, transform->position - direction, glm::vec3(0, 1, 0));
	transform->rotation = glm::quat_cast(transformView);

	lastMouseX = mouseX;
	lastMouseY = mouseY;
}