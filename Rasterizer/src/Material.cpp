#include "Material.h"
#include "ColorRGB.h"

namespace dae
{
	Material::Material(Texture* albedoPtr, Texture* normalPtr, Texture* specularPtr, Texture* glossPtr)
		: m_AlbedoPtr{ albedoPtr }
		, m_NormalPtr{ normalPtr }
		, m_SpecularPtr{ specularPtr }
		, m_GlossPtr{ glossPtr }
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

			const ColorRGB sampledNormal{ m_NormalPtr->Sample(vertex.uv) };
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

	ColorRGB Material::GetSpecular(const Vertex_Out& vertex, const Vector3& lightDirection, const ColorRGB& ambientColor) const
	{
		float specular{ 1.f }; 
		float phongExponent{ 25.f }; // shininess

		if (m_SpecularPtr != nullptr)
		{
			try
			{
				specular *= m_SpecularPtr->Sample(vertex.uv).r;
			}
			catch (...)
			{
				// color is already set to white
			}
		}
		
		if (m_GlossPtr)
		{
			try
			{
				phongExponent *= m_GlossPtr->Sample(vertex.uv).r;
			}
			catch (...)
			{
				// exponent is already set to 2
			}
		}

		return BRDF::Phong(	ambientColor, specular, phongExponent, 
						lightDirection, vertex.viewDirection, vertex.normal);
	}

	Material_SolidColor::Material_SolidColor(Texture* albedoPtr, Texture* normalPtr)
		: Material(albedoPtr, normalPtr, nullptr, nullptr)
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

	Material_Lambert::Material_Lambert(	Texture* albedoPtr, Texture* normalPtr, 
										Texture* specularPtr, Texture* glossPtr, float diffuseReflectance)
		: Material(albedoPtr, normalPtr, specularPtr, glossPtr)
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
