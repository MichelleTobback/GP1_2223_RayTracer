//External includes
#include "SDL.h"
#include "SDL_surface.h"

#include <thread>
#include <future> //async stuff
#include <ppl.h> //parallel stuff

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

//#define ASYNC
#define PARALLEL_FOR

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

	float aspectRatio{ float(m_Width) / float(m_Height) };
	float angleRad{ PI / 180.f * camera.fovAngle };
	float fov{ tanf(angleRad / 2.f) };

	const uint32_t numPixels{ uint32_t(m_Width * m_Height) };

#if defined(ASYNC)
	//Async logic
	const uint32_t numCores{ std::thread::hardware_concurrency()};
	std::vector<std::future<void>> async_futures{};

	const uint32_t numPixelsPerTask{ numPixels / numCores };
	uint32_t numUnassignedPixels{ numPixels % numCores };
	uint32_t currentPixelIndex{ 0 };

	//create tasks
	for (uint32_t coreId{}; coreId < numCores; ++coreId)
	{
		uint32_t taskSize{ numPixelsPerTask };
		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(
			std::async(std::launch::async, [=, this] 
				{
					const uint32_t pixelIndexEnd{currentPixelIndex + taskSize};
					for (uint32_t pixelIndex{ currentPixelIndex }; pixelIndex < pixelIndexEnd; pixelIndex++)
					{
						RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
					}
				})
		);

		currentPixelIndex += taskSize;
	}

	//Wait for all tasks
	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	//parallel logic
	concurrency::parallel_for(0u, numPixels, [=, this](int i)
		{
			RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
		});

#else

	for (uint32_t i{}; i < numPixels; i++)
	{
		RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
	}

#endif


	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

Vector3 Renderer::RasterSpaceToCameraSpace(float x, float y, int width, int height, float aspectRatio, float fov) const
{
	x += 0.5f;
	y += 0.5f;

	Vector3 result{};
	result.x = ((2.f * x / float(width)) - 1.f) * (aspectRatio * fov);
	result.y = (1.f - (2.f * y / float(height))) * fov;
	result.z = 1.f;

	return result.Normalized();
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& pMaterials) const
{
	const uint32_t px{ pixelIndex % m_Width };
	const uint32_t py{ pixelIndex / m_Width };

	Vector3 rayDirection{ RasterSpaceToCameraSpace(float(px), float(py), m_Width, m_Height, aspectRatio, fov) };
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
			Vector3 directionToLight{ light.origin - hitStats.origin };
			Ray toLightRay{};
			toLightRay.max = directionToLight.Normalize();
			toLightRay.min = 0.001f;

			toLightRay.direction = directionToLight;
			toLightRay.origin = hitStats.origin;

			bool canSeeLight{ (m_ShadowEnabled) ? !pScene->DoesHit(toLightRay) : true };

			ColorRGB brdfRgb{ pMaterials[hitStats.materialIndex]->Shade(hitStats, directionToLight, -rayDirection) };

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
					finalColor += ColorRGB{ 1.f, 1.f, 1.f } *cosTheta;
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
