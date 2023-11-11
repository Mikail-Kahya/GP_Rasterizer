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

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	const int nrTrigVertices{ 3 };

	m_AspectRatio = static_cast<float>(m_Width) / m_Height;
	m_TrigVertexVec.resize(nrTrigVertices);
	m_AreaParallelVec.resize(nrTrigVertices);

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
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

	UpdateBuffer();

	// Setup vertices for screen space
	std::vector<Vertex> vertices_world
	{
		// Triangle 1
		{{0.f, 2.f, 0.f}, {1, 0, 0}},
		{{1.5f, -1.f, 0.f}, {1, 0, 0}},
		{{-1.5f, -1.f, 0.f}, {1, 0 ,0}},
		// Triangle 2
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

		// Calculate area of triangle
		const Vector2 edge1{ m_TrigVertexVec[1] - m_TrigVertexVec[0] };
		const Vector2 edge2{ m_TrigVertexVec[2] - m_TrigVertexVec[0] };
		const float areaTrig{ Vector2::Cross(edge1, edge2) / 2 };

		const Rect boundingBox{ GetBoundingBox(m_TrigVertexVec) };

		// Clamp bounding box to not be any negative values (out of screen)
		const int startX{ std::clamp(boundingBox.x, 0, m_Width) };
		const int startY{ std::clamp(boundingBox.y, 0, m_Height) };
		const int endX{ std::clamp(boundingBox.x + boundingBox.width, 0, m_Width) };
		const int endY{ std::clamp(boundingBox.y + boundingBox.height, 0, m_Height) };

		for (int px{ startX }; px < endX; ++px)
		{
			const float screenX{ px + 0.5f };

			for (int py{ startY }; py < endY; ++py)
			{
				ColorRGB finalColor{ };
				float pixelDepth{};

				const float screenY{ py + 0.5f };
				const Vector3 pixelPos{ screenX, screenY, 1 };

				// Checks whether or not the pixel is in the triangle and fills areaParallelVec
				const bool inTriangle{ GeometryUtils::PixelInTriangle(m_TrigVertexVec, pixelPos, m_AreaParallelVec) };

				if (!inTriangle)
					continue;

				// Figure out the depth and color of a pixel on an object (barycentric coordinates reversed)
				for (int interpolateIdx{}; interpolateIdx < nrTrigVertices; ++interpolateIdx)
				{
					const float weight{ (m_AreaParallelVec[interpolateIdx] * 0.5f) / areaTrig };
					const Vertex& vertex{ screenSpaceVec[(trigIdx * nrTrigVertices + interpolateIdx)] };

					finalColor += vertex.color * weight;
					pixelDepth += vertex.position.z * weight;
				}
				if (AddPixelToDepthBuffer(pixelDepth, px, py))
					AddPixelToRGBBuffer(finalColor, px, py);
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

		// Add perspective
		vertex.x /= vertex.z;
		vertex.y /= vertex.z;

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

void Renderer::UpdateBuffer()
{
	const int nrPixels{ m_Width * m_Height };
	for (int idx{}; idx < nrPixels; ++idx)
		m_pDepthBufferPixels[idx] = std::numeric_limits<float>::max();

	SDL_FillRect(m_pBackBuffer, nullptr, GetSDLRGB(m_ClearColor));
}

void Renderer::AddPixelToRGBBuffer(ColorRGB& color, int x, int y) const
{
	//Update Color in Buffer
	color.MaxToOne();

	m_pBackBufferPixels[x + (y * m_Width)] = GetSDLRGB(color);
}

bool Renderer::AddPixelToDepthBuffer(float depth, int x, int y) const
{
	const int idx{ x + (y * m_Width) };
	const bool isCloser{ m_pDepthBufferPixels[idx] >= depth };
	if (isCloser)
		m_pDepthBufferPixels[idx] = depth;

	return isCloser;
}

Uint32 Renderer::GetSDLRGB(const ColorRGB& color) const
{
	return SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(color.r * 255),
		static_cast<uint8_t>(color.g * 255),
		static_cast<uint8_t>(color.b * 255));
}

Rect Renderer::GetBoundingBox(const std::vector<Vector3>&vertexVec) const
{
	Vector2 bottomLeft{ vertexVec[0] };
	Vector2 topRight{ vertexVec[0] };

	for (const Vector3& vertex : vertexVec)
	{
		bottomLeft.x = std::min(bottomLeft.x, vertex.x);
		bottomLeft.y = std::min(bottomLeft.y, vertex.y);
		topRight.x = std::max(topRight.x, vertex.x);
		topRight.y = std::max(topRight.y, vertex.y);
	}

	return Rect{ bottomLeft, topRight };
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
