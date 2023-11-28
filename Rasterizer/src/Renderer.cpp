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
	m_TriangleVertices.resize(NR_TRI_VERTS);
	m_AreaParallelVec.resize(NR_TRI_VERTS);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Render()
{
	assert(m_ScenePtr != nullptr && "No scene found");

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

void Renderer::RenderMesh(const Mesh& mesh)
{
	int nrTris{};

	switch (mesh.primitiveTopology)
	{
	case PrimitiveTopology::TriangleList:
		nrTris = static_cast<int>(mesh.indices.size()) / NR_TRI_VERTS;
		break;
	case PrimitiveTopology::TriangleStrip:
		nrTris = static_cast<int>(mesh.indices.size()) - 2;
		break;
	}

	for (int triIdx{}; triIdx < nrTris; ++triIdx)
	{
		// Strip degenerate check
		if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip &&
			IsDegenerate(mesh.indices))
			continue;

		// Fills the triangle vertices in m_TriangleVertices
		const TriangleIndices indices{ GetIndices(mesh, triIdx) };
		FillTriangle(mesh.vertices_out, indices);

		// Frustum culling
		if (!InFrustum(m_TriangleVertices))
			continue;

		if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip &&
			triIdx % 2 != 0)
			std::swap(m_TriangleVertices[1], m_TriangleVertices[2]);

		// NDC to screen space
		for (Vertex_Out& vertex : m_TriangleVertices)
			vertex.position = NDCToScreenSpace(vertex.position);

		RenderTriangle(mesh.texturePtr);
	}
}

void Renderer::RenderTriangle(Texture* texturePtr)
{
	// Calculate area of triangle
	const Vector2 edge1{ m_TriangleVertices[1].position - m_TriangleVertices[0].position };
	const Vector2 edge2{ m_TriangleVertices[2].position - m_TriangleVertices[0].position };
	const float areaTri{ Vector2::Cross(edge1, edge2) / 2 };

	const Rect boundingBox{ GeometryUtils::GetBoundingBox({
		m_TriangleVertices[0].position,
		m_TriangleVertices[1].position,
		m_TriangleVertices[2].position
		})
	};
	ColorRGB finalColor{};

	// Clamp bounding box to screen
	const int startX{ std::clamp(boundingBox.x, 0, m_Width) };
	const int startY{ std::clamp(boundingBox.y, 0, m_Height) };
	const int endX{ std::clamp(boundingBox.x + boundingBox.width, 0, m_Width) };
	const int endY{ std::clamp(boundingBox.y + boundingBox.height, 0, m_Height) };

	for (int px{ startX }; px < endX; ++px)
	{
		const float screenX{ px + 0.5f };

		for (int py{ startY }; py < endY; ++py)
		{
			float zDepth{};
			float wDepth{};
			Vector2 UVCoord{};

			const float screenY{ py + 0.5f };
			const Vector3 pixelPos{ screenX, screenY, 1 };

			// Checks whether or not the pixel is in the triangle and fills areaParallelVec
			const bool inTriangle{ GeometryUtils::PixelInTriangle(m_TriangleVertices, pixelPos, m_AreaParallelVec) };

			if (!inTriangle)
				continue;

			// Figure out the depth and color of a pixel on an object (barycentric coordinates reversed)
			for (int interpolateIdx{}; interpolateIdx < NR_TRI_VERTS; ++interpolateIdx)
			{
				const int oppositeIdx{ (interpolateIdx + 2) % NR_TRI_VERTS };
				const float weight{ (m_AreaParallelVec[interpolateIdx] * 0.5f) / areaTri };
				const Vertex_Out& vertex{ m_TriangleVertices[oppositeIdx] };
				const float newWDepth{ 1 / vertex.position.w * weight };

				finalColor += vertex.color * weight;
				wDepth += newWDepth;
				zDepth += 1 / vertex.position.z * weight;
				UVCoord += vertex.uv * newWDepth;
			}

			// Done for proper depth buffer
			zDepth = 1 / zDepth;
			wDepth = 1 / wDepth;
			UVCoord *= wDepth;

			// Depth view mode
			const float depthColor{ Remap(0.8f, 1.f, zDepth) };

			switch (m_RenderMode)
			{
			case RenderMode::Texture:
				assert(texturePtr != nullptr && "No texture found");
				finalColor = texturePtr->Sample(UVCoord);
				break;
			case RenderMode::Depth:
				finalColor = ColorRGB{ depthColor, depthColor, depthColor };
				break;
			}

			if (AddPixelToDepthBuffer(zDepth, px, py))
				AddPixelToRGBBuffer(finalColor, px, py);
		}
	}
}

void Renderer::VerticesTransform(std::vector<Mesh>& meshVec) const
{
	const Camera& camera{ m_ScenePtr->GetCamera() };

	for (Mesh& mesh : meshVec)
	{
		const Matrix worldViewProjectionMatrix{ mesh.worldMatrix * camera.worldToCamera * camera.projectionMatrix };

		for (Vertex& vertex : mesh.vertices)
			mesh.vertices_out.push_back(VertexTransform(vertex, worldViewProjectionMatrix));
	}
}

Vertex_Out Renderer::VertexTransform(const Vertex& vertex_in, const Matrix& worldViewProjectionMatrix) const
{
	//Todo > W1 Projection Stage
	Vector4 pos{ worldViewProjectionMatrix.TransformPoint({vertex_in.position, 1}) };

	// Add perspective
	pos.x /= pos.w;
	pos.y /= pos.w;
	pos.z /= pos.w;

	return {
		pos,
		vertex_in.color,
		vertex_in.uv,
		vertex_in.normal,
		vertex_in.tangent,
		vertex_in.viewDirection
	};
}

Vector4 Renderer::NDCToScreenSpace(const Vector4& NDC) const
{
	Vector4 screenSpace{ NDC };

	screenSpace.x = (NDC.x + 1) * 0.5f * m_Width;
	screenSpace.y = (1 - NDC.y) * 0.5f * m_Height;

	return screenSpace;
}

void Renderer::FillTriangle(const TriangleVertices& vertices, const TriangleIndices& indices)
{
	for (int vertexIdx{}; vertexIdx < NR_TRI_VERTS; ++vertexIdx)
		m_TriangleVertices[vertexIdx] = vertices[indices[vertexIdx]];
}

float Renderer::Remap(float min, float max, float value) const
{
	const float range{ max - min };
	const float multiplier{ 1 / range };
	return (min - value) * multiplier;
}

void Renderer::UpdateBuffer()
{
	SDL_FillRect(m_pBackBuffer, nullptr, GetSDLRGB(m_ClearColor));
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
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

int Renderer::GetNrStrips(const std::vector<uint32_t>& indices) const
{
	int nrOfDoubles{};
	int prevNr{ -1 };

	for (uint32_t idx : indices)
	{
		if (prevNr == idx)
			++nrOfDoubles;
		prevNr = idx;
	}

	// there will always be two doubles per degenerate tri so divide by 2
	return nrOfDoubles / 2;
}

Renderer::TriangleIndices Renderer::GetIndices(const Mesh& mesh, int triIdx) const
{
	TriangleIndices indexArr{};

	switch (mesh.primitiveTopology)
	{
	case PrimitiveTopology::TriangleList:
		for (int vertexIdx{}; vertexIdx < NR_TRI_VERTS; ++vertexIdx)
			indexArr[vertexIdx] = mesh.indices[triIdx * NR_TRI_VERTS + vertexIdx];
		break;
	case PrimitiveTopology::TriangleStrip:
		for (int vertexIdx{}; vertexIdx < NR_TRI_VERTS; ++vertexIdx)
			indexArr[vertexIdx] = mesh.indices[triIdx + vertexIdx];
		break;
	}

	return indexArr;
}

bool Renderer::IsDegenerate(const std::vector<uint32_t>& indexVec)
{
	int prevNr{ -1 };

	for (uint32_t index : indexVec)
	{
		if (prevNr == index)
			return true;

		prevNr = index;
	}

	return false;
}

bool Renderer::InFrustum(const TriangleVertices& vertexVec)
{
	for (const Vertex_Out& vertex : vertexVec)
	{
		if (!InRange(0.f, 1.f, vertex.position.z))
			return false;

		if (!InRange(-1.f, 1.f, vertex.position.x))
			return false;

		if (!InRange(-1.f, 1.f, vertex.position.y))
			return false;
	}

	return true;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void Renderer::SetScene(Scene* scenePtr, SDL_Window* windowPtr)
{
	scenePtr->GetCamera().aspectRatio = m_AspectRatio;
	m_ScenePtr = scenePtr;
	SDL_SetWindowTitle(windowPtr, ("Rasterizer (Mikail Kahya 2GD10) - " + scenePtr->GetName()).c_str());
}

void Renderer::CycleRenderMode()
{
	constexpr int endIdx{ static_cast<int>(RenderMode::end) };
	int modeIdx{ static_cast<int>(m_RenderMode) };
	modeIdx = ++modeIdx % endIdx;
	m_RenderMode = static_cast<RenderMode>(modeIdx);
}
