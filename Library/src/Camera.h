#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
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
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			viewMatrix = Matrix{	{right, 0},
									{up, 0},
									{forward, 0},
									{origin, 1}
								};
			invViewMatrix = viewMatrix.Inverse();

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W3

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			//...

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			// mouseState: left == 1, middle == 2, right == 3
			switch (mouseState)
			{
			case SDL_BUTTON(1):
				MouseMove(mouseY, deltaTime);
				Rotate(mouseX, 0, deltaTime);
				break;
			case SDL_BUTTON(3):
				Move(pKeyboardState, mouseState, mouseY, deltaTime);
				Rotate(mouseX, mouseY, deltaTime);
				break;
			}

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

	private:
		void Move(const uint8_t* keyboardStatePtr, uint32_t mouseState, int mouseY, float deltaTime)
		{
			const float moveSpeed{ 10.f };
			const float movedDistance{ moveSpeed * deltaTime };

			// WASD in camera view
			if (keyboardStatePtr[SDL_SCANCODE_W])
				origin += forward * movedDistance;

			if (keyboardStatePtr[SDL_SCANCODE_S])
				origin -= forward * movedDistance;

			if (keyboardStatePtr[SDL_SCANCODE_A])
				origin -= right * movedDistance;

			if (keyboardStatePtr[SDL_SCANCODE_D])
				origin += right * movedDistance;

			// Up and down
			if (keyboardStatePtr[SDL_SCANCODE_Q])
				origin -= up * movedDistance;

			if (keyboardStatePtr[SDL_SCANCODE_E])
				origin += up * movedDistance;
		}

		void Rotate(int mouseX, int mouseY, float deltaTime)
		{
			const float MouseSpeed{ 20.f };
			bool mouseMoved{ false };

			if (mouseX)
			{
				totalYaw += mouseX * deltaTime * MouseSpeed;
				mouseMoved = true;
			}
			if (mouseY)
			{
				totalPitch += mouseY * deltaTime * MouseSpeed;
				totalPitch = std::clamp(totalPitch, -90.f, 90.f);
				mouseMoved = true;
			}

			if (mouseMoved)
			{
				const Matrix finalTransform{ Matrix::CreateRotation(TO_RADIANS * totalPitch, TO_RADIANS * totalYaw, 0) };
				forward = finalTransform.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}
		}

		void MouseMove(int mouseY, float deltaTime)
		{
			const float moveSpeed{ 10.f };
			const float movedDistance{ moveSpeed * deltaTime };

			if (mouseY < 0)
				origin += forward * movedDistance;
			else if (mouseY > 0)
				origin -= forward * movedDistance;
		}
	};
}
