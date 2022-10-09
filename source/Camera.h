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

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			cameraToWorld = { right, up, forward, origin };
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float movementSpeed{ 8.f };
			const float rotSpeed{PI_DIV_4 / 2.f};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			bool hasMoved{ false };
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += (forward * movementSpeed * deltaTime);
				hasMoved = true;
			}
			else if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= (forward * movementSpeed * deltaTime);
				hasMoved = true;
			}

			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += (right * movementSpeed * deltaTime);
				hasMoved = true;
			}
			else if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= (right * movementSpeed * deltaTime);
				hasMoved = true;
			}

			bool hasRotated{false};
			if (mouseState & SDL_BUTTON(1) && mouseState & SDL_BUTTON(3))
			{
				origin -= (up * movementSpeed * deltaTime * float(mouseY));
				hasMoved = true;
			}
			else
			{
				if (mouseState & SDL_BUTTON(3))
				{
					totalYaw += float(mouseX) * rotSpeed * deltaTime;
					totalPitch -= float(mouseY) * rotSpeed * deltaTime;
					hasRotated = true;
					hasMoved = true;
				}
				else if (mouseState & SDL_BUTTON(1))
				{
					totalYaw += float(mouseX) * rotSpeed * deltaTime;
					origin += forward * -float(mouseY) * movementSpeed * deltaTime;
					hasRotated = true;
					hasMoved = true;
				}
			}
			
			if (totalPitch >= PI_2)
				totalPitch = 0.f;
			if (totalPitch < 0.f)
				totalPitch = PI_2;
			if (totalYaw >= PI_2)
				totalYaw = 0.f;
			if (totalYaw < 0.f)
				totalYaw = PI_2;

			if (hasRotated)
			{
				Matrix rotMat{ Matrix::CreateRotation(totalPitch, totalYaw, 0.f) };
				forward = rotMat.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}
			if (hasMoved)
			{
				CalculateCameraToWorld();
			}
		}	

	};
}
