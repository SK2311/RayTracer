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

#include <future> //async
#include<ppl.h> //parallel_for

using namespace dae;

//#define ASYNC
#define PARALLEL_FOR

Renderer::Renderer(SDL_Window* pWindow) :
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
	camera.CalculateCameraToWorld();

	float aspectRatio{ m_Width / static_cast<float>(m_Height) };
	float fov{ std::tanf((camera.fovAngle * TO_RADIANS) / 2.f) };

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t numPixels = m_Width * m_Height;


#if defined(ASYNC)
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};
	const uint32_t numPixelsPerTask = numPixels / numCores;
	uint32_t numUnassignedPixels = numPixels % numCores;
	uint32_t currPixelIndex = 0;

	for (uint32_t coreId{}; coreId < numCores; ++coreId)
	{
		uint32_t taskSize = numPixelsPerTask;
		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}
		//= is to acces all the locals
		//this is to acces the member variables
		async_futures.push_back(std::async(std::launch::async, [=, this]
			{
				const uint32_t pixelIndexEnd = currPixelIndex + taskSize;
				for (uint32_t pixelIndex{ currPixelIndex }; pixelIndex < pixelIndexEnd; ++pixelIndex)
				{
					RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
				}
			}));

		currPixelIndex += taskSize;		
	}

	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	Concurrency::parallel_for(0u, numPixels, [=, this](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
		});
#else
	for (uint32_t i{}; i < numPixels; ++i)
	{
		RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
	}
#endif
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	//Convert from raster space to camera space
	float cx = (((2 * (px + 0.5f) / (float) m_Width) - 1) * aspectRatio) * fov;
	float cy = (1 - (2 * (py + 0.5f) / (float) m_Height)) * fov;

	Vector3 rayDirection{ cx,cy,1 };
	rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();

	Ray viewRay{ camera.origin, rayDirection };

	ColorRGB finalColor{};

	HitRecord closestHit{};

	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		for (int i{ 0 }; i < lights.size(); ++i)
		{
			//cache the directionToLight to be used in multiple LightingModes 
			//prevents multiple calls to the same function
			Vector3 directionToLightFunction{ LightUtils::GetDirectionToLight(lights[i], closestHit.origin) };
			float distance = directionToLightFunction.Normalize();

			Ray invLightRay{ closestHit.origin, directionToLightFunction, 0.001f, distance };
			if (m_ShadowsEnabled && pScene->DoesHit(invLightRay)) continue;

			switch (m_CurrentLightingMode)
			{
			case dae::Renderer::LightingMode::ObservedArea:
			{
				//calculate the observed area lighting with Lambert's cosine law
				float observedArea = Vector3::Dot(closestHit.normal, directionToLightFunction);
				if (observedArea < 0)
					continue;

				finalColor += ColorRGB{ observedArea, observedArea, observedArea };
				break;
			}
			case dae::Renderer::LightingMode::Radiance:
			{
				auto lightRadiance{ LightUtils::GetRadiance(lights[i], closestHit.origin) };

				finalColor += lightRadiance;
				break;
			}
			case dae::Renderer::LightingMode::BRDF:
			{
				ColorRGB BRDFColour{ materials[closestHit.materialIndex]->Shade(closestHit, -directionToLightFunction, rayDirection) };

				finalColor += BRDFColour;
				break;
			}
			case dae::Renderer::LightingMode::Combined:
			{
				float observedArea = Vector3::Dot(closestHit.normal, directionToLightFunction);
				if (observedArea < 0)
					continue;

				//inverse direction to get the correct direction from the light to the point
				//we originally calculate from the point to the light
				ColorRGB BRDFColour{ materials[closestHit.materialIndex]->Shade(closestHit, -directionToLightFunction, rayDirection) };
				auto lightRadiance{ LightUtils::GetRadiance(lights[i], closestHit.origin) };
				finalColor += lightRadiance * BRDFColour * ColorRGB{ observedArea, observedArea, observedArea };
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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		std::cout << "Change to Radiance\n";
		break;
	case dae::Renderer::LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		std::cout << "Change to BRDF\n";
		break;
	case dae::Renderer::LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		std::cout << "Change to Combined\n";
		break;
	case dae::Renderer::LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		std::cout << "Change to ObservedArea\n";
		break;
	default:
		break;
	}
}
