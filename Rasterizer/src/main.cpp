//External includes
#include "vld.h"
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include <list>

#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneManager.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void SetTitle(SDL_Window* windowPtr, const std::string& title)
{
	SDL_SetWindowTitle(windowPtr, ("Rasterizer (Mikail Kahya 2GD10) - " + title).c_str());
}

void PrintInfo()
{
	std::cout << "------------------------------\n";
	std::cout << "Cycle through scene: F3\n";
	std::cout << "Toggle depth buffer: F4\n";
	std::cout << "Toggle rotation: F5\n";
	std::cout << "Toggle normals: F6\n";
	std::cout << "Cycle shading mode: F7\n";
	std::cout << "\n--- Print info: I\n";
	std::cout << "--- Clear console: C\n";
	std::cout << "------------------------------" << std::endl;
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"Rasterizer - W6 DEMO",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	SceneManager sceneManager{};

	//Start loop
	pTimer->Start();
	pRenderer->SetScene(sceneManager.GetScene());
	SetTitle(pWindow, sceneManager.GetScene()->GetName());

	PrintInfo();
	// Start Benchmark
	// TODO pTimer->StartBenchmark();

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;

				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
					pRenderer->CycleRenderMode();

				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					Scene* scenePtr{ sceneManager.Next() };
					pRenderer->SetScene(scenePtr);
					SetTitle(pWindow, scenePtr->GetName());
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
					pRenderer->ToggleDepthMode();

				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
					sceneManager.GetScene()->ToggleMeshUpdates();

				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
					pRenderer->ToggleNormalMode();

				if (e.key.keysym.scancode == SDL_SCANCODE_I)
					PrintInfo();

				if (e.key.keysym.scancode == SDL_SCANCODE_C)
					system("CLS");

				break;
			}
		}

		//--------- Update ---------
		sceneManager.Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}