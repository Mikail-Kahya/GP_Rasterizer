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
		float aspectRatio{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix worldToCamera{};
		Matrix projectionMatrix{};

		// in degrees
		float rotateSpeed{ 1.5f };
		float moveSpeed{ 20.f };

		float zFar{ 100.f };
		float zNear{ 5.f };

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

			worldToCamera = Matrix::CreateLookAtLH(origin, forward, up, right);

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W3
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, zNear, zFar);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime{ pTimer->GetElapsed() };

			//Camera Update Logic
			//...

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};

			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const bool leftPress{ static_cast<bool>(mouseState & SDL_BUTTON_LMASK) };
			const bool rightPress{ static_cast<bool>(mouseState & SDL_BUTTON_RMASK) };

			// Only left mouse
			if (leftPress && !rightPress)
			{
				MouseMove(mouseY, deltaTime);
				Rotate(mouseX * 0.1f, 0);
			}

			// Only right mouse
			if (rightPress && !leftPress)
			{
				Move(pKeyboardState, mouseState, mouseY, deltaTime);
				Rotate(mouseX, mouseY);
			}

			// Left and right mouse
			if (leftPress && rightPress && mouseY)
			{
				VerticalMove(-mouseY / abs(mouseY), deltaTime);
			}

			//Update Matrices
			CalculateViewMatrix();

			// Only update if the ratio or fov changes
			if (RatiosChanged())
				CalculateProjectionMatrix();

			oldFov = fov;
			oldAspectRatio = aspectRatio;
		}

	private:
		float oldFov{};
		float oldAspectRatio{};

		void Move(const uint8_t* keyboardStatePtr, uint32_t mouseState, int mouseY, float deltaTime)
		{
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

		void Rotate(int mouseX, int mouseY)
		{
			bool mouseMoved{ false };

			if (mouseX)
			{
				totalYaw += mouseX * rotateSpeed;
				totalYaw = std::fmod(totalYaw, 360.f);
				mouseMoved = true;
			}
			if (mouseY)
			{
				// Flor allowed this :)
				totalPitch -= mouseY * rotateSpeed;
				totalPitch = std::clamp(totalPitch, -90.f, 90.f);
				mouseMoved = true;
			}

			if (mouseMoved)
			{
				const Matrix finalTransform{ Matrix::CreateRotation(TO_RADIANS * totalPitch, TO_RADIANS * totalYaw, 0) };
				forward = finalTransform.TransformPoint(Vector3::UnitZ);
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

		void VerticalMove(int direction, float deltaTime)
		{
			const float movedDistance{ moveSpeed * deltaTime };
			origin += up * direction * movedDistance;
		}

		bool RatiosChanged() const
		{
			if (!AreEqual(fov, oldFov))
				return true;

			return !AreEqual(aspectRatio, oldAspectRatio);
		}
	};
}
