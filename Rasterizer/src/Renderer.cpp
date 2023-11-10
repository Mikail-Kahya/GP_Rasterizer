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

	m_AspectRatio = static_cast<float>(m_Width) / m_Height;

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

	std::vector<Vertex> vertices_world
	{
		{{0.f, 2.f, 0.f}},
		{{1.f, 0.f, 0.f}},
		{{-1.f, 0.f, 0.f}}
	};
	std::vector<Vertex> screenSpaceVec{};
	screenSpaceVec.resize(vertices_world.size());
	VertexTransformationFunction(vertices_world, screenSpaceVec);

	ColorRGB finalColor{ };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			const bool inTriangle{ GeometryUtils::PixelInTriangle(screenSpaceVec,
																	Vector2{ static_cast<float>(px), static_cast<float>(py) }) };
			finalColor = (inTriangle) ? colors::White : colors::Black;

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
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
		vertex /= vertex.z;

		vertex.x /= m_Camera.fov * m_AspectRatio;
		vertex.y /= m_Camera.fov;
			
		vertex.x = (vertex.x + 1) * 0.5f * m_Width + 0.5f;
		vertex.y = (1 - vertex.y) * 0.5f * m_Height + 0.5f;

		vertexVec_out[idx].position = vertex;
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
