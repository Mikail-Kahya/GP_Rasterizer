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

		void SetScene(Scene* scenePtr);
		void ToggleRenderType();

	private:
		// constants to prevent retyping
		static constexpr int NR_TRI_VERTS{ 3 };

		// Rendering functions
		void RenderMesh(const Mesh& mesh);
		void RenderTriangle(Texture* texturePtr);

		void VerticesTransform(std::vector<Mesh>& meshVec) const;
		Vertex_Out VertexTransform(const Vertex& vertex_in, const Matrix& worldViewProjectionMatrix) const;

		// Buffer functions
		void UpdateBuffer();
		void AddPixelToRGBBuffer(ColorRGB& color, int x, int y) const;
		bool AddPixelToDepthBuffer(float depth, int x, int y) const;
		float Remap(float min, float max, float value) const;

		// Helpers
		Uint32 GetSDLRGB(const ColorRGB& color) const;
		Rect GetBoundingBox() const noexcept;

		bool IsDegenerate(const Mesh& mesh, int triIdx);
		bool InFrustum(const Mesh& mesh, int triIdx);

		int GetNrStrips(const std::vector<uint32_t>& indices) const;
		std::array<uint32_t, NR_TRI_VERTS> GetIndices(const Mesh& mesh, int triIdx) const;
		void FillTriangle(const Mesh& mesh, int triIdx);

		SDL_Window* m_pWindow{};
		Scene* m_ScenePtr{};
		bool m_RenderColor{ true };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		
		ColorRGB m_ClearColor{0.2f, 0.2f, 0.2f};
		float* m_pDepthBufferPixels{};

		float m_AspectRatio{};

		int m_Width{};
		int m_Height{};

		// Vectors here to prevent allocation on every frame
		std::vector<Vertex_Out> m_TriangleVertexVec{};
		std::vector<float> m_AreaParallelVec{};
	};
}
