#include "Material.h"
#include "ColorRGB.h"

namespace dae
{
	Material::Material(Texture* albedoPtr, Texture* normalPtr)
		: m_AlbedoPtr{ albedoPtr }
		, m_NormalPtr{ normalPtr }
	{
	}

	Vector3 Material::GetNormal(const Vertex_Out& vertex) const
	{
		if (m_NormalPtr == nullptr)
			return vertex.normal;

		try
		{
			const Vector3 binormal{ Vector3::Cross(vertex.normal, vertex.tangent) };
			const Matrix tangentSpaceAxis{
				vertex.tangent,
				binormal,
				vertex.normal,
				Vector3::Zero
			};

			const ColorRGB sampledNormal{ m_NormalPtr->Sample(vertex.uv) / 255.f };
			return  tangentSpaceAxis.TransformPoint({
				2.f * sampledNormal.r - 1.f,
				2.f * sampledNormal.g - 1.f,
				2.f * sampledNormal.b - 1.f,
				});
		}
		catch (...)
		{
			return vertex.normal;
		}
	}

	Material_SolidColor::Material_SolidColor(Texture* albedoPtr, Texture* normalPtr)
		: Material(albedoPtr, normalPtr)
	{
	}

	ColorRGB Material_SolidColor::GetAlbedo(const Vertex_Out& vertex) const
	{
		if (m_AlbedoPtr == nullptr)
			return vertex.color;

		try
		{
			return m_AlbedoPtr->Sample(vertex.uv);
		}
		catch (...)
		{
			return vertex.color;
		}
	}

	Material_Lambert::Material_Lambert(Texture* albedoPtr, Texture* normalPtr, float diffuseReflectance)
		: Material(albedoPtr, normalPtr)
		, m_DiffuseReflectance{ diffuseReflectance }
	{
	}

	ColorRGB Material_Lambert::GetAlbedo(const Vertex_Out& vertex) const
	{
		if (m_AlbedoPtr == nullptr)
			return vertex.color;

		try
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_AlbedoPtr->Sample(vertex.uv));
		}
		catch (...)
		{
			return vertex.color;
		}
	}
}
