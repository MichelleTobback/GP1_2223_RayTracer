//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//float gradient = px / static_cast<float>(m_Width);
			//gradient += py / static_cast<float>(m_Width);
			//gradient /= 2.0f;
			//
			//ColorRGB finalColor{ gradient, gradient, gradient };

			Vector3 direction{ RasterSpaceToCameraSpace( float(px), float(py), m_Width, m_Height ) };
			Ray hitRay{ {0.f, 0.f, 0.f}, direction };

			ColorRGB finalColor{};
			HitRecord hitStats{};

			pScene->GetClosestHit(hitRay, hitStats);

			if (hitStats.didHit)
			{
				finalColor = materials[hitStats.materialIndex]->Shade();
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

Vector3 Renderer::RasterSpaceToCameraSpace(float x, float y, int width, int height) const
{
	x += 0.5f;
	y += 0.5f;

	Vector3 result{};
	result.x = ((2.f * x / float(width)) - 1.f) * (float(width) / float(height));
	result.y = 1.f - (2.f * y / float(height));
	result.z = 1.f;

	return result.Normalized();
}
