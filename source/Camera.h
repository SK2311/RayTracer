#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		float movementSpeed{1.f};
		float rotationSpeed{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			//assert(false && "Not Implemented Yet");

			Vector3 newRight = Vector3::Cross(Vector3::UnitY, forward);
			newRight.Normalize();

			Vector3 newUp = Vector3::Cross(forward, newRight);
			newUp.Normalize();

			cameraToWorld = { newRight, newUp, forward, origin };
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_W])
				origin.z += (movementSpeed * deltaTime);

			if (pKeyboardState[SDL_SCANCODE_A])
				origin.x += (-movementSpeed * deltaTime);

			if (pKeyboardState[SDL_SCANCODE_S])
				origin.z += (-movementSpeed * deltaTime);

			if (pKeyboardState[SDL_SCANCODE_D])
				origin.x += (movementSpeed * deltaTime);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
