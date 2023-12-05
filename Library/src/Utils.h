#pragma once
#include <array>
#include <cassert>
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

//#define DISABLE_OBJ

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Triangle HitTest	
		//TRIANGLE HIT-TESTS
		inline bool PixelInTriangle(const std::vector<Vertex_Out>& vertexVec, const Vector3& pixelPos, std::vector<float>& areaParallelVec, bool clockWise = true)
		{
			const int nrVertices{ static_cast<int>(vertexVec.size()) };

			for (int idx{}; idx < nrVertices; ++idx)
			{
				const int idx2{ (idx + 1) % nrVertices };

				const Vector2 vertexToPixel{ vertexVec[idx].position - pixelPos };
				const Vector2 edge{ vertexVec[idx].position - vertexVec[idx2].position };
				const float areaParallel = (clockWise) ? Vector2::Cross(edge, vertexToPixel) : Vector2::Cross(vertexToPixel, edge);

				if (areaParallel < 0)
					return false;

				areaParallelVec[idx] = areaParallel;
			}

			return true;
		}

		inline float Remap(float min, float max, float value)
		{
			const float range{ max - min };
			const float multiplier{ 1 / range };
			return (min - value) * multiplier;
		}

		inline Uint32 GetSDLRGB(SDL_Surface* bufferPtr, const ColorRGB& color)
		{
			return SDL_MapRGB(bufferPtr->format,
				static_cast<uint8_t>(color.r * 255),
				static_cast<uint8_t>(color.g * 255),
				static_cast<uint8_t>(color.b * 255));
		}

		inline Rect GetBoundingBox(const std::vector<Vector4>& vertices) noexcept
		{
			if (vertices.empty())
				return {};

			constexpr int margin{ 2 };
			Vector2 bottomLeft{ vertices[0] };
			Vector2 topRight{ vertices[0] };

			for (const Vector4& vertex : vertices)
			{
				bottomLeft.x = std::min(bottomLeft.x, vertex.x - margin);
				bottomLeft.y = std::min(bottomLeft.y, vertex.y - margin);
				topRight.x = std::max(topRight.x, vertex.x + margin);
				topRight.y = std::max(topRight.y, vertex.y + margin);
			}

			return Rect{ bottomLeft, topRight };
		}

		inline bool InFrustum(const std::vector<Vertex_Out>& vertices)
		{
			for (const Vertex_Out& vertex : vertices)
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

		inline bool IsDegenerate(const std::vector<uint32_t>& indexVec)
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

#pragma endregion
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
#ifdef DISABLE_OBJ

			//TODO: Enable the code below after uncommenting all the vertex attributes of DataTypes::Vertex
			// >> Comment/Remove '#define DISABLE_OBJ'
			assert(false && "OBJ PARSER not enabled! Check the comments in Utils::ParseOBJ");

#else

			std::ifstream file(filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 vertices, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					Vertex vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.uv = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(vertices.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding) 
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[size_t(i) + 1];
				uint32_t index2 = indices[size_t(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].uv;
				const Vector2& uv1 = vertices[index1].uv;
				const Vector2& uv2 = vertices[index2].uv;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Fix the tangents per vertex now because we accumulated
			for (auto& v : vertices)
			{
				v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

				if(flipAxisAndWinding)
				{
					v.position.z *= -1.f;
					v.normal.z *= -1.f;
					v.tangent.z *= -1.f;
				}

			}

			return true;
#endif
		}
#pragma warning(pop)
	}
}