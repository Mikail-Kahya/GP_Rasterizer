#pragma once
#include <cassert>

#include "Math.h"
#include "vector"

namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};

		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}
		Triangle(const std::vector<Vector3>& vertices, const Vector3& normal):
			Triangle(vertices[0], vertices[1], vertices[2], normal) {}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode):
		positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			//Calculate Normals
			CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), normals(_normals), cullMode(_cullMode)
		{
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		Vector3 minAABB{};
		Vector3 maxAABB{};

		Vector3 transformedMinAABB{};
		Vector3 transformedMaxAABB{};

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if(!ignoreTransformUpdate)
				UpdateTransforms();
		}

		void CalculateNormals()
		{
			constexpr int nrVertices{ 3 };
			const int nrTriangles{ static_cast<int>(indices.size() / nrVertices) };

			for (int index{}; index < nrTriangles; ++index)
			{
				const int offset{ index * nrVertices };

				const Vector3 v0{ positions[indices[offset]] };
				const Vector3 v1{ positions[indices[offset + 1]] };
				const Vector3 v2{ positions[indices[offset + 2]] };

				normals.push_back(Vector3::Cross(v1 - v0, v2 - v1));
			}
		}

		void UpdateTransforms()
		{
			ReserveSpace();

			//Calculate Final Transform 
			//const auto finalTransform = ...
			const Matrix finalTransform{ scaleTransform * rotationTransform * translationTransform };

			//Transform Positions (positions > transformedPositions)
			//...
			for (int index{}; index < static_cast<int>(positions.size()); ++index)
				transformedPositions[index] = finalTransform.TransformPoint(positions[index]);

			//Transform Normals (normals > transformedNormals)
			//...
			for (int index{}; index < static_cast<int>(normals.size()); ++index)
				transformedNormals[index] = rotationTransform.TransformVector(normals[index]);

			UpdateAABB();
			UpdateTransformedAABB(finalTransform);
		}

		void ReserveSpace()
		{
			if (transformedNormals.size() != normals.size())
				transformedNormals.resize(normals.size());
			if (transformedPositions.size() != positions.size())
				transformedPositions.resize(positions.size());
		}

		void UpdateAABB()
		{
			if (!positions.empty())
			{
				minAABB = positions[0];
				maxAABB = positions[0];
				for (const Vector3& pos : positions)
				{
					minAABB = Vector3::Min(pos, minAABB);
					maxAABB = Vector3::Max(pos, maxAABB);
				}
			}
		}

		void UpdateTransformedAABB(const Matrix& finalTransform)
		{
			Vector3 tMinAABB{ finalTransform.TransformPoint(minAABB) };
			Vector3 tMaxAABB{ tMinAABB };

			// xMax / yMin / zMin
			Vector3 tAABB{ finalTransform.TransformPoint(maxAABB.x,minAABB.y,minAABB.z) };
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// xMax / yMin / zMax
			tAABB = finalTransform.TransformPoint(maxAABB.x,minAABB.y,maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// xMin / yMin / zMax
			tAABB = finalTransform.TransformPoint(minAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// xMax / yMax / zMin
			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// xMax / yMax / zMin
			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// xMax / yMax / zMax
			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// xMin / yMax / zMax
			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, maxAABB.z);
			transformedMinAABB = Vector3::Min(tAABB, tMinAABB);
			transformedMaxAABB = Vector3::Max(tAABB, tMaxAABB);
		}
	};
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };
	};
#pragma endregion
}