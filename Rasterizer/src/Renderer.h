#pragma once

#include <cstdint>
#include <vector>
#include <array>

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

		void SetScene(Scene* scenePtr, SDL_Window* windowPtr);
		void CycleRenderMode();

	private:
		// constants to prevent retyping
		static constexpr int NR_TRI_VERTS{ 3 };
		typedef std::array<uint32_t, NR_TRI_VERTS> TriangleIndices;
		typedef std::vector<Vertex_Out> TriangleVertices;

		enum class RenderMode
		{
			Texture,
			Depth,
			Color,
			end
		};

		// Rendering functions
		void RenderMesh(const Mesh& mesh);
		void RenderTriangle(Texture* texturePtr);

		void VerticesTransform(std::vector<Mesh>& meshVec) const;
		Vertex_Out VertexTransform(const Vertex& vertex_in, const Matrix& worldViewProjectionMatrix) const;

		Vector4 NDCToScreenSpace(const Vector4& NDC) const;
		void FillTriangle(const TriangleVertices& vertices, const TriangleIndices& indices);
		float Remap(float min, float max, float value) const;

		// Buffer functions
		void UpdateBuffer();
		void AddPixelToRGBBuffer(ColorRGB& color, int x, int y) const;
		bool AddPixelToDepthBuffer(float depth, int x, int y) const;

		// Helpers
		Uint32 GetSDLRGB(const ColorRGB& color) const;
		int GetNrStrips(const std::vector<uint32_t>& indexVec) const;
		TriangleIndices GetIndices(const Mesh& mesh, int triIdx) const;

		bool IsDegenerate(const std::vector<uint32_t>& indexVec);
		bool InFrustum(const TriangleVertices& vertices);
		 
		SDL_Window* m_pWindow{};
		Scene* m_ScenePtr{};
		RenderMode m_RenderMode{ RenderMode::Texture };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		
		ColorRGB m_ClearColor{0.2f, 0.2f, 0.2f};
		float* m_pDepthBufferPixels{};

		float m_AspectRatio{};

		int m_Width{};
		int m_Height{};

		// Vectors here to prevent allocation on every frame
		TriangleVertices m_TriangleVertices{};
		std::vector<float> m_AreaParallelVec{};
	};
}
