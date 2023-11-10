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

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

		void VertexTransformationFunction(const std::vector<Vertex>& vertexVec_in, std::vector<Vertex>& vertexVec_out) const;

	private:
		void Render_W6_Part1() const;
		void Render_W6_Part2();
		void Render_W6_Part3() const;
		void Render_W6_Part4() const;
		void Render_W6_Part5() const;

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		//float* m_pDepthBufferPixels{};
		float m_AspectRatio{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};
	};
}
