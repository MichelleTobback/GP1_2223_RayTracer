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
			Vector3 direction{ RasterSpaceToCameraSpace( float(px), float(py), m_Width, m_Height, camera.fovAngle ) };
			direction = camera.cameraToWorld.TransformVector(direction);
			
			Ray hitRay{ camera.origin, direction };

			ColorRGB finalColor{};
			HitRecord hitStats{};

			pScene->GetClosestHit(hitRay, hitStats);

			if (hitStats.didHit)
			{
				finalColor = materials[hitStats.materialIndex]->Shade();

				for (const Light& light : lights)
				{
					Ray toLightRay{};
					Vector3 directionToLight{light.origin - hitStats.origin};
					toLightRay.max = directionToLight.Normalize();
					Vector3 toLightOrigin{ hitStats.origin + directionToLight * toLightRay.min};

					toLightRay.direction = directionToLight;
					toLightRay.origin = toLightOrigin;
					
					if (pScene->DoesHit(toLightRay))
					{
						finalColor *= 0.5f;
					}
				}
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

Vector3 Renderer::RasterSpaceToCameraSpace(float x, float y, int width, int height, float fovAngle) const
{
	x += 0.5f;
	y += 0.5f;

	float aspectRatio{ float(width) / float(height) };
	float angleRad{ PI / 180.f * fovAngle };
	float fov{tanf(angleRad / 2.f)};

	Vector3 result{};
	result.x = ((2.f * x / float(width)) - 1.f) * (aspectRatio * fov);
	result.y = (1.f - (2.f * y / float(height))) * fov;
	result.z = 1.f;

	return result.Normalized();
}
