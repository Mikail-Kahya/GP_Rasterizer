#include "Texture.h"

#include <cassert>

#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		SDL_Surface* surfacePtr{ IMG_Load(path.c_str()) };

		assert(surfacePtr == nullptr && "No image found with the path: " + path);
		//Create & Return a new Texture Object (using SDL_Surface)

		return new Texture{ surfacePtr };
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv
		const int idx{ static_cast<int>(uv.x * uv.y) };

		assert(idx < 0 && "Sampling out of range");

		const uint32_t sdlColor{ m_pSurfacePixels[idx] };

		return {
			(sdlColor & 255) / 255.f,
			(sdlColor >> 8 & 255) / 255.f,
			(sdlColor >> 16 & 255) / 255.f
		};
	}
}