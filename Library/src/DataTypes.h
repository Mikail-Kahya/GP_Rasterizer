#pragma once
#include "Maths.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		ColorRGB color{colors::White};
		//Vector2 uv{}; //W2
		//Vector3 normal{}; //W4
		//Vector3 tangent{}; //W4
		//Vector3 viewDirection{}; //W4
	};

	struct Vertex_Out
	{
		Vector4 position{};
		ColorRGB color{ colors::White };
		//Vector2 uv{};
		//Vector3 normal{};
		//Vector3 tangent{};
		//Vector3 viewDirection{};
	};

	struct Triangle
	{
		Triangle() = default;

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			//const Vector3 edgeV0V1 = _v1 - _v0;
			//const Vector3 edgeV0V2 = _v2 - _v0;
			//normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}
		Triangle(const std::vector<Vector3>& vertices) :
			Triangle(vertices[0], vertices[1], vertices[2]) {}


		Vertex v0{};
		Vertex v1{};
		Vertex v2{};
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};


	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{};
	};

	struct TriangleMesh
	{
		std::vector<Triangle> triangleVec{};
		Matrix worldMatrix{};
	};
	
}
