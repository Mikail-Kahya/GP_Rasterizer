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
		const int x{ static_cast<int>(m_pSurface->w * uv.x) % m_pSurface->w };
		const int y{ static_cast<int>(m_pSurface->h * uv.y) & m_pSurface->h };

		uint8_t r, g, b;

		SDL_GetRGB(m_pSurfacePixels[x * y], m_pSurface->format, &r, &g, &b);

		return {
			r / 255.f ,
			g / 255.f,
			b / 255.f
		};
	}
}