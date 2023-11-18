//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include "Scene.h"
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

	m_AspectRatio = static_cast<float>(m_Width) / m_Height;
	m_TrigVertexVec.resize(NR_TRI_VERTS);
	m_AreaParallelVec.resize(NR_TRI_VERTS);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	UpdateBuffer();

	// Setup vertices for screen space
	for (Mesh& mesh : m_ScenePtr->GetMeshes())
		mesh.vertices_out.clear();

	VerticesTransform(m_ScenePtr->GetMeshes());

	for (const Mesh& mesh : m_ScenePtr->GetMeshes())
		RenderMesh(mesh);

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VerticesTransform(std::vector<Mesh>& meshVec) const
{
	for (Mesh& mesh : meshVec)
	{
		for (Vertex& vertex : mesh.vertices)
			mesh.vertices_out.push_back(VertexTransform(vertex));
		
		const int nrTris{ static_cast<int>(mesh.indices.size()) / NR_TRI_VERTS };

		for (int triIdx{}; triIdx < nrTris; ++triIdx)
		{
			const int vertexOffset{ NR_TRI_VERTS * triIdx };
			const int vertexEnd{ vertexOffset + NR_TRI_VERTS };

			for(int indicesIdx{ vertexOffset }; indicesIdx < vertexEnd; ++indicesIdx)
			{
				// Get vertex from indices
				const uint32_t vertexIdx{ mesh.indices[indicesIdx] };
				Vertex_Out vertex{ VertexTransform(mesh.vertices[vertexIdx]) };
				mesh.vertices_out.push_back(std::move(vertex));
			}
		}
	}
}

Vertex_Out Renderer::VertexTransform(const Vertex& vertex_in) const
{
	//Todo > W1 Projection Stage
	const Camera& camera{ m_ScenePtr->GetCamera() };

	Vector4 pos{ camera.worldToCamera.TransformPoint({vertex_in.position, 0}) };

	// Add perspective
	pos.x /= pos.z;
	pos.y /= pos.z;

	// Account for screen dimensions and fov
	pos.x /= camera.fov * m_AspectRatio;
	pos.y /= camera.fov;

	// NDC (Normalized Device Coordinates) ===> Screen space
	pos.x = (pos.x + 1) * 0.5f * m_Width;
	pos.y = (1 - pos.y) * 0.5f * m_Height;

	return { pos, vertex_in.color };
}

void Renderer::RenderMesh(const Mesh& mesh)
{
	int nrVertices{};

	switch (mesh.primitiveTopology)
	{
	case PrimitiveTopology::TriangleList:
		nrVertices = static_cast<int>(mesh.indices.size());
		break;
	case PrimitiveTopology::TriangleStrip:
		nrVertices = GetNrStripVertices(mesh.indices);
		break;
	}

	//const int nrVertices{ static_cast<int>(mesh.indices.size()) };
	const int nrTrigs{ nrVertices / NR_TRI_VERTS };

	for (int trigIdx{}; trigIdx < nrTrigs; ++trigIdx)
	{
		const bool isEven{ trigIdx % 2 == 0 };
		int vertexOffset{trigIdx * NR_TRI_VERTS };

		// Fill up the current triangle
		for (int vertexIdx{}; vertexIdx < NR_TRI_VERTS; ++vertexIdx)
		{
			const uint32_t indicesIdx{ mesh.indices[trigIdx * NR_TRI_VERTS + vertexIdx] };
			m_TrigVertexVec[vertexIdx] = mesh.vertices_out[indicesIdx].position;
		}

		// Calculate area of triangle
		const Vector2 edge1{ m_TrigVertexVec[1] - m_TrigVertexVec[0] };
		const Vector2 edge2{ m_TrigVertexVec[2] - m_TrigVertexVec[0] };
		const float areaTri{ Vector2::Cross(edge1, edge2) / 2 };

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
				for (int interpolateIdx{}; interpolateIdx < NR_TRI_VERTS; ++interpolateIdx)
				{
					const float weight{ (m_AreaParallelVec[interpolateIdx] * 0.5f) / areaTri };
					const Vertex_Out& vertex{ mesh.vertices_out[(trigIdx * NR_TRI_VERTS + interpolateIdx)] };

					finalColor += vertex.color * weight;
					pixelDepth += vertex.position.z * weight;
				}
				if (AddPixelToDepthBuffer(pixelDepth, px, py))
					AddPixelToRGBBuffer(finalColor, px, py);
			}
		}
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

int Renderer::GetNrStripVertices(const std::vector<uint32_t>& indices) const
{
	int nrOfStrips{};
	int prevNr{ -1 };

	for (uint32_t idx : indices)
	{
		if (prevNr == idx)
			++nrOfStrips;
		prevNr = idx;
	}

	// there will always be two doubles per degenerate tri so divide by 2
	return static_cast<int>(indices.size()) - nrOfStrips / 2;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
