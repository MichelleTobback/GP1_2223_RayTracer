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
			Vector3 rayDirection{ RasterSpaceToCameraSpace( float(px), float(py), m_Width, m_Height, camera.fovAngle ) };
			rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
			
			Ray hitRay{ camera.origin, rayDirection };

			ColorRGB finalColor{};
			HitRecord hitStats{};

			pScene->GetClosestHit(hitRay, hitStats);

			if (hitStats.didHit)
			{
				finalColor = { 0.f, 0.f, 0.f };

				for (const Light& light : lights)
				{
					Vector3 directionToLight{light.origin - hitStats.origin};
					Ray toLightRay{};
					toLightRay.max = directionToLight.Normalize();
					toLightRay.min = 0.001f;

					toLightRay.direction = directionToLight;
					toLightRay.origin = hitStats.origin;

					bool canSeeLight{ (m_ShadowEnabled) ? !pScene->DoesHit(toLightRay) : true };

					ColorRGB brdfRgb{ materials[hitStats.materialIndex]->Shade(hitStats, directionToLight, -rayDirection)};

					if (canSeeLight)
					{
						float cosTheta{ Vector3::Dot(hitStats.normal, directionToLight) }; 
						if (cosTheta < 0.f)
						{
							cosTheta = 0.f;
						}

						switch (m_LightingMode)
						{
						case LightingMode::ObservedArea:
							finalColor += ColorRGB{ 1.f, 1.f, 1.f } * cosTheta;
							break;

						case LightingMode::Radiance:
							finalColor += LightUtils::GetRadiance(light, hitStats.origin);
							break;

						case LightingMode::BRDF:
							finalColor += brdfRgb;
							break;

						case LightingMode::Combined:
							ColorRGB radiance{ LightUtils::GetRadiance(light, hitStats.origin) };
							finalColor += radiance * brdfRgb * cosTheta;
							break;
						}
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
