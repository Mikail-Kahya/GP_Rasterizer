#pragma once
#include <cassert>

#include "ColorRGB.h"
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			return cd * kd / PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			return cd * kd / PI;
		}

		static float ObservedArea(const Vector3& lightDirection, const Vector3& normal)
		{
			// Use inverted light (hit to light)
			return std::max(0.f, Vector3::Dot(-lightDirection, normal));
		}
	}
}