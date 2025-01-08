#pragma once

#include "glm/vec3.hpp"
#include "glm/ext/matrix_transform.hpp"

enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class MCamera
{
public:
	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	// euler Angles
	float Yaw = -90.f;
	float Pitch = 0.f;
	// camera options
	float MovementSpeed = 10.f;
	float MouseSensitivity = 0.1f;
	float Zoom = 45.f;

	MCamera();

	MCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

	MCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	inline glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

private:
	void UpdateCameraVectors();

};