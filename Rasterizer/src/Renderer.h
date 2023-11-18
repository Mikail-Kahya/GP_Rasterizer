#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	struct Vector3;
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render();

		bool SaveBufferToImage() const;

		void VertexTransformationFunction(const std::vector<Vertex>& vertexVec_in, std::vector<Vertex>& vertexVec_out) const;
		void SetScene(Scene* scenePtr) { m_ScenePtr = scenePtr; }

	private:
		void UpdateBuffer();
		void AddPixelToRGBBuffer(ColorRGB& color, int x, int y) const;
		bool AddPixelToDepthBuffer(float depth, int x, int y) const;
		Uint32 GetSDLRGB(const ColorRGB& color) const;
		Rect GetBoundingBox(const std::vector<Vector3>& vertexVec) const;

		SDL_Window* m_pWindow{};
		Scene* m_ScenePtr{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		
		ColorRGB m_ClearColor{};
		float* m_pDepthBufferPixels{};

		float m_AspectRatio{};

		int m_Width{};
		int m_Height{};

		// Vectors here to prevent allocation on every frame
		std::vector<Vector3> m_TrigVertexVec{};
		std::vector<float> m_AreaParallelVec{};
	};
}
