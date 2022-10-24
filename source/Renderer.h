#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;
	struct Vector3;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		void ToggleShadows() { m_ShadowEnabled = !m_ShadowEnabled; }
		void ToggleLightingMode() { m_LightingMode = (int(m_LightingMode) < 3) ? LightingMode(int(m_LightingMode) + 1) : LightingMode(0); }

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		enum class LightingMode
		{
			ObservedArea = 0,
			Radiance = 1,
			BRDF = 2,
			Combined = 3
		};

		LightingMode m_LightingMode{ LightingMode::Combined };
		bool m_ShadowEnabled{ true };

		Vector3 RasterSpaceToCameraSpace(float x, float y, int width, int height, float fovAngle) const;

	};
}
