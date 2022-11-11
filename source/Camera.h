#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}

		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		float movementSpeed{ 5.f };

		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			//assert(false && "Not Implemented Yet");

			//create rotation matrix to calculate forward/right/up vector
			//create and return OBN matrix
			Matrix rotation = Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationY(totalYaw * TO_RADIANS);
			forward = rotation.GetAxisZ();
			right = rotation.GetAxisX();
			up = rotation.GetAxisY();

			cameraToWorld = { right, up, forward, origin };
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W])
				origin += forward * movementSpeed * deltaTime;

			if (pKeyboardState[SDL_SCANCODE_A])
				origin += right  *  (- movementSpeed) * deltaTime;

			if (pKeyboardState[SDL_SCANCODE_S])
				origin += -forward * movementSpeed * deltaTime;

			if (pKeyboardState[SDL_SCANCODE_D])
				origin += right * movementSpeed * deltaTime;

			if (pKeyboardState[SDL_SCANCODE_LEFT])
				fovAngle += 5.f;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				origin += up * (float)-mouseY * deltaTime;
				CalculateCameraToWorld();
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				const float yaw = (float)mouseX;
				totalYaw += yaw;

				origin += forward * (float) - mouseY * movementSpeed * deltaTime;

				CalculateCameraToWorld();
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				const float pitch = (float) - mouseY;
				const float yaw = (float) mouseX;

				totalPitch += pitch;
				totalYaw += yaw;

				CalculateCameraToWorld();
			}

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
