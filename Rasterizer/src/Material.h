#pragma once
#include "Math.h"
#include "DataTypes.h"
#include "BRDFs.h"
#include "Texture.h"

namespace dae
{
#pragma region Material BASE
	// Materials should house the texture used
	// Textures are not created in the material and kept in the scene

	// The texture will be checked (catch)
		// If there is no texture, use the interpolated color
	// Solid color displays the texture

	class Material
	{
	public:
		Material(Texture* texturePtr) : m_TexturePtr{ texturePtr } {}
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;

		virtual ColorRGB PixelShade(const Vertex_Out& vertex) = 0;

	protected:
		Texture* m_TexturePtr{ nullptr };
	};
#pragma endregion

#pragma region Material SOLID COLOR
	//SOLID COLOR
	//===========
	class Material_SolidColor final : public Material
	{
	public:
		Material_SolidColor(Texture* texturePtr = nullptr) : Material(texturePtr) {}

		ColorRGB PixelShade(const Vertex_Out& vertex) override
		{
			ColorRGB finalColor{};
			try
			{
				finalColor = m_TexturePtr->Sample(vertex.uv);
			}
			catch(...)
			{
				finalColor = vertex.color;
			}
			return finalColor;
			
		}

	private:
		ColorRGB m_Color{colors::White};
	};
#pragma endregion

#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final : public Material
	{
	public:
		Material_Lambert(float diffuseReflectance) : Material_Lambert(nullptr, diffuseReflectance){}
		Material_Lambert(Texture* texturePtr, float diffuseReflectance) :
			Material(texturePtr), m_DiffuseReflectance(diffuseReflectance){}

		ColorRGB PixelShade(const Vertex_Out& vertex) override
		{
			ColorRGB finalColor{};

			try
			{
				finalColor = m_TexturePtr->Sample(vertex.uv);
			}
			catch (...)
			{
				finalColor = vertex.color;
			}

			return BRDF::Lambert(m_DiffuseReflectance, finalColor);
		}

	private:
		float m_DiffuseReflectance{1.f}; //kd
	};
#pragma endregion
}
