//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	const int nrTrigVertices{ 3 };

	m_AspectRatio = static_cast<float>(m_Width) / m_Height;
	m_TrigVertexVec.resize(nrTrigVertices);
	m_AreaParallelVec.resize(nrTrigVertices);

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	// Setup vertices for screen space
	std::vector<Vertex> vertices_world
	{
		{{0.f, 4.f, 2.f}, {1, 0, 0}},
		{{3.f, -2.f, 2.f}, {0, 1, 0}},
		{{-3.f, -2.f, 2.f}, {0, 0 ,1}}
	};
	std::vector<Vertex> screenSpaceVec{};
	screenSpaceVec.resize(vertices_world.size());
	VertexTransformationFunction(vertices_world, screenSpaceVec);

	const int nrTrigVertices{ 3 };
	const int nrTrigs{ static_cast<int>(vertices_world.size()) / nrTrigVertices };

	for (int trigIdx{}; trigIdx < nrTrigs; ++trigIdx)
	{
		// Fill up the current triangle
		for (int vertexIdx{}; vertexIdx < nrTrigVertices; ++vertexIdx)
			m_TrigVertexVec[vertexIdx] = screenSpaceVec[trigIdx * nrTrigVertices + vertexIdx].position;

		const Vector2 edge1{ m_TrigVertexVec[1] - m_TrigVertexVec[0] };
		const Vector2 edge2{ m_TrigVertexVec[2] - m_TrigVertexVec[0] };
		const float areaTrig{ Vector2::Cross(edge1, edge2) / 2 };

		for (int px{}; px < m_Width; ++px)
		{
			const float screenX{ px + 0.5f };

			for (int py{}; py < m_Height; ++py)
			{
				ColorRGB finalColor{ };

				const float screenY{ py + 0.5f };
				const Vector3 pixelPos{ screenX, screenY, 1 };

				// Checks whether or not the pixel is in the triangle and fills areaParallelVec
				const bool inTriangle{ GeometryUtils::PixelInTriangle(m_TrigVertexVec, pixelPos, m_AreaParallelVec) };

				if (!inTriangle)
				{
					AddPixelToBuffer(finalColor, px, py);
					continue;
				}

				// Add color
				for (int colorIdx{}; colorIdx < nrTrigVertices; ++colorIdx)
				{
					const float weight{ (m_AreaParallelVec[colorIdx] * 0.5f) / areaTrig };
					finalColor += screenSpaceVec[(colorIdx + 2) % nrTrigVertices].color * weight;
				}

				AddPixelToBuffer(finalColor, px, py);
			}
		}
	}

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertexVec_in, std::vector<Vertex>& vertexVec_out) const
{
	//Todo > W1 Projection Stage
	const int nrVertices{ static_cast<int>(vertexVec_in.size()) };
	for (int idx{}; idx < nrVertices; ++idx)
	{
		Vector3 vertex{ m_Camera.invViewMatrix.TransformPoint(vertexVec_in[idx].position) };

		// might need to change in the future not to divide the z component
		// Add perspective
		vertex /= vertex.z;

		// Account for screen dimensions and fov
		vertex.x /= m_Camera.fov * m_AspectRatio;
		vertex.y /= m_Camera.fov;

		// NDC (Normalized Device Coordinates) ===> Screen space
		vertex.x = (vertex.x + 1) * 0.5f * m_Width;
		vertex.y = (1 - vertex.y) * 0.5f * m_Height;

		vertexVec_out[idx].position = vertex;
		vertexVec_out[idx].color = vertexVec_in[idx].color;
	}
}

void Renderer::AddPixelToBuffer(ColorRGB& color, int x, int y) const
{
	//Update Color in Buffer
	color.MaxToOne();

	m_pBackBufferPixels[x + (y * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(color.r * 255),
		static_cast<uint8_t>(color.g * 255),
		static_cast<uint8_t>(color.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
