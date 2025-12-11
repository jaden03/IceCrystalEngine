#include <Ice/Components/Camera.h>
#include <array>

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
	
	// Prevent division by zero - use 16:9 as fallback if window not initialized
	float aspectRatio = 16.0f / 9.0f;
	if (windowManager.windowWidth > 0 && windowManager.windowHeight > 0)
	{
		aspectRatio = (float)windowManager.windowWidth / (float)windowManager.windowHeight;
	}
	
	projection = glm::perspective(glm::radians(fieldOfView), aspectRatio, nearClippingPlane, farClippingPlane);

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

std::array<glm::vec4, 8> Camera::GetFrustomCornersWorldSpace(float nearDist, float farDist)
{
	const float aspect = (float)windowManager.windowWidth / (float)windowManager.windowHeight;
	const float tanFov = glm::tan(glm::radians(fieldOfView) / 2.0f);

	float nh = tanFov * nearDist;
	float nw = nh * aspect;
	float fh = tanFov * farDist;
	float fw = fh * aspect;

	glm::vec3 forward = transform->forward;
	glm::vec3 right   = transform->right;
	glm::vec3 up      = transform->up;

	glm::vec3 nc = transform->position + forward * nearDist;
	glm::vec3 fc = transform->position + forward * farDist;

	return std::array<glm::vec4, 8>{
		glm::vec4(nc + up*nh - right*nw, 1.0f),
		glm::vec4(nc + up*nh + right*nw, 1.0f),
		glm::vec4(nc - up*nh - right*nw, 1.0f),
		glm::vec4(nc - up*nh + right*nw, 1.0f),

		glm::vec4(fc + up*fh - right*fw, 1.0f),
		glm::vec4(fc + up*fh + right*fw, 1.0f),
		glm::vec4(fc - up*fh - right*fw, 1.0f),
		glm::vec4(fc - up*fh + right*fw, 1.0f)
	};
}