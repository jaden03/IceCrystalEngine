#include <Ice/Components/Camera.h>

Camera::Camera() : Component()
{
}

Camera::Camera(float fieldOfView) : Component()
{
	this->fieldOfView = fieldOfView;
}

Camera::Camera(float fieldOfView, float nearClippingPlane, float farClippingPlane) : Component()
{
	this->fieldOfView = fieldOfView;
	this->nearClippingPlane = nearClippingPlane;
	this->farClippingPlane = farClippingPlane;
}

void Camera::Update()
{
	view = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);
	projection = glm::perspective(glm::radians(fieldOfView), (float)windowManager.windowWidth / (float)windowManager.windowHeight, nearClippingPlane, farClippingPlane);

	frustumCorners = GetFrustomCorners();
	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& corner : frustumCorners)
	{
		center += glm::vec3(corner);
	}
	center /= frustumCorners.size();
	frustumCenter = center;
}

std::vector<glm::vec4> Camera::GetFrustomCorners()
{
	const auto inv = glm::inverse(projection * view);

	std::vector<glm::vec4> corners;
	for (unsigned int x = 0; x < 2; x++)
	{
		for (unsigned int y = 0; y < 2; y++)
		{
			for (unsigned int z = 0; z < 2; z++)
			{
				const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				corners.push_back(pt / pt.w);
			}
		}
	}
	
	return corners;
}
