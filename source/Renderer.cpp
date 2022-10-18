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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float screenWidth{ static_cast<float>(m_Width) };
	float screenHeight{ static_cast<float>(m_Height) };
	float aspectRatio{ screenWidth / screenHeight };
	float fov{ std::tanf((camera.fovAngle * TO_RADIANS) / 2.f) };
	camera.CalculateCameraToWorld();

	bool isInShadow{ false };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//Convert from raster space to camera space
			float cx = (((2 * (px + 0.5f) / screenWidth) - 1) * aspectRatio) * fov;
			float cy = (1 - (2 * (py + 0.5f) / screenHeight)) * fov;

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

					/*if (m_ShadowsEnabled)
					{
						Vector3 directionToLight = directionToLightFunction;
						const float directionMagnitude = directionToLight.Normalize();
						Ray ray{ closestHit.origin + (closestHit.normal * 0.001f), directionToLight };
						ray.max = directionMagnitude;
						if (pScene->DoesHit(ray))
						{
							finalColor *= 0.9f;
						}
					}*/
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
