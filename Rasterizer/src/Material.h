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
		Material(Texture* albedoPtr, Texture* normalPtr, Texture* specularPtr, Texture* glossPtr);
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;

		virtual ColorRGB GetAlbedo(const Vertex_Out& vertex) const = 0;
		virtual Vector3 GetNormal(const Vertex_Out& vertex) const;
		virtual ColorRGB GetSpecular(const Vertex_Out& vertex, const Vector3& lightDirection, const ColorRGB& ambientColor = colors::White) const;

	protected:
		Texture* m_AlbedoPtr{ nullptr };
		Texture* m_NormalPtr{ nullptr };
		Texture* m_SpecularPtr{ nullptr };
		Texture* m_GlossPtr{ nullptr };
	};
#pragma endregion

#pragma region Material SOLID
	class Material_SolidColor final : public Material
	{
	public:
		Material_SolidColor(Texture* albedoPtr, Texture* normalPtr = nullptr);
		virtual ~Material_SolidColor() override = default;

		Material_SolidColor(const Material_SolidColor&) = delete;
		Material_SolidColor(Material_SolidColor&&) noexcept = delete;
		Material_SolidColor& operator=(const Material_SolidColor&) = delete;
		Material_SolidColor& operator=(Material_SolidColor&&) noexcept = delete;

		ColorRGB GetAlbedo(const Vertex_Out& vertex) const override;

	};
#pragma endregion


#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final : public Material
	{
	public:
		Material_Lambert(Texture* albedoPtr, Texture* normalPtr = nullptr, Texture* specularPtr = nullptr, Texture* glossPtr = nullptr, float diffuseReflectance = 1.f);
		virtual ~Material_Lambert() override = default;

		Material_Lambert(const Material_Lambert&) = delete;
		Material_Lambert(Material_Lambert&&) noexcept = delete;
		Material_Lambert& operator=(const Material_Lambert&) = delete;
		Material_Lambert& operator=(Material_Lambert&&) noexcept = delete;

		ColorRGB GetAlbedo(const Vertex_Out& vertex) const override;

	private:
		float m_DiffuseReflectance{1.f}; //kd
	};
#pragma endregion
}
