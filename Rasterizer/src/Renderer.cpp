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
	m_TriangleVertexVec.resize(NR_TRI_VERTS);
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
		if (!InFrustum(mesh, triIdx))
			continue;

		// Fill up the current triangle
		switch (mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleList:
			FillTriangle(mesh, triIdx);
			break;
		case PrimitiveTopology::TriangleStrip:
			// Skip degenerate triangles
			if (IsDegenerate(mesh, triIdx))
				continue;

			FillTriangle(mesh, triIdx);

			if (triIdx % 2 != 0)
				std::swap(m_TriangleVertexVec[1], m_TriangleVertexVec[2]);
			break;
		}

		for (Vertex_Out& vertex : m_TriangleVertexVec)
			vertex.position = NDCToScreenSpace(vertex.position);

		RenderTriangle(mesh.texturePtr);
	}
}

void Renderer::RenderTriangle(Texture* texturePtr)
{
	// Calculate area of triangle
	const Vector2 edge1{ m_TriangleVertexVec[1].position - m_TriangleVertexVec[0].position };
	const Vector2 edge2{ m_TriangleVertexVec[2].position - m_TriangleVertexVec[0].position };
	const float areaTri{ Vector2::Cross(edge1, edge2) / 2 };

	const Rect boundingBox{ GetBoundingBox() };
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
			const bool inTriangle{ GeometryUtils::PixelInTriangle(m_TriangleVertexVec, pixelPos, m_AreaParallelVec) };

			if (!inTriangle)
				continue;

			// Figure out the depth and color of a pixel on an object (barycentric coordinates reversed)
			for (int interpolateIdx{}; interpolateIdx < NR_TRI_VERTS; ++interpolateIdx)
			{
				const int oppositeIdx{ (interpolateIdx + 2) % NR_TRI_VERTS };
				const float weight{ (m_AreaParallelVec[interpolateIdx] * 0.5f) / areaTri };
				const Vertex_Out& vertex{ m_TriangleVertexVec[oppositeIdx] };
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

void Renderer::FillTriangle(const Mesh& mesh, int triIdx)
{
	std::array indexArr{ GetIndices(mesh, triIdx) };
	for (int vertexIdx{}; vertexIdx < NR_TRI_VERTS; ++vertexIdx)
		m_TriangleVertexVec[vertexIdx] = mesh.vertices_out[indexArr[vertexIdx]];
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

Rect Renderer::GetBoundingBox() const noexcept
{
	if (m_TriangleVertexVec.empty())
		return {};

	constexpr int margin{ 2 };
	Vector2 bottomLeft{ m_TriangleVertexVec[0].position };
	Vector2 topRight{ m_TriangleVertexVec[0].position };

	for (const Vertex_Out& vertex : m_TriangleVertexVec)
	{
		bottomLeft.x = std::min(bottomLeft.x, vertex.position.x - margin);
		bottomLeft.y = std::min(bottomLeft.y, vertex.position.y - margin);
		topRight.x = std::max(topRight.x, vertex.position.x + margin);
		topRight.y = std::max(topRight.y, vertex.position.y + margin);
	}

	return Rect{ bottomLeft, topRight };
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

std::array<uint32_t, Renderer::NR_TRI_VERTS> Renderer::GetIndices(const Mesh& mesh, int triIdx) const
{
	std::array<uint32_t, NR_TRI_VERTS> indexArr{};

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

bool Renderer::IsDegenerate(const Mesh& mesh, int triIdx)
{
	int prevNr{ -1 };

	for (int vertexIdx{}; vertexIdx < NR_TRI_VERTS; ++vertexIdx)
	{
		const int offset{ triIdx + vertexIdx };

		if (prevNr == mesh.indices[offset])
			return true;

		prevNr = mesh.indices[offset];
	}

	return false;
}

bool Renderer::InFrustum(const Mesh& mesh, int triIdx)
{
	int prevNr{ -1 };
	const float fWidth{ static_cast<float>(m_Width) };
	const float fHeight{ static_cast<float>(m_Height) };
	std::array indexArr{ GetIndices(mesh, triIdx) };

	for (int vertexIdx{}; vertexIdx < NR_TRI_VERTS; ++vertexIdx)
	{
		const Vertex_Out& vertex{ mesh.vertices_out[indexArr[vertexIdx]] };

		if (!InRange(-1.f, 1.f, vertex.position.z))
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
