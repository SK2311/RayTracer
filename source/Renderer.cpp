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
				Vector3 distanceToLightVector{};
				float distanceToLight{};
				for (auto& light : pScene->GetLights())
				{
					distanceToLightVector = light.origin - closestHit.origin;
					distanceToLight = distanceToLightVector.Normalize();
					
					for (const dae::Sphere& sphere : pScene->GetSphereGeometries())
					{
						Vector3 normal = sphere.origin - closestHit.origin;
						normal.Normalize();
						//move closestHit.origin along the normal by 0.1f and put into movedHit
						//to avoid self-shadowing
						Vector3 movedHit = closestHit.origin + (normal * 0.1f);
						Ray lightRay = { movedHit, distanceToLightVector, 0.001f, distanceToLight };
						if (pScene->DoesHit(lightRay))
						{
							//occluder between light and sphere
							if (lightRay.max < distanceToLight)
							{
								//shadow
								isInShadow = false;
								
							}
							else if (lightRay.max > distanceToLight || lightRay.max == distanceToLight)
							{
								//lit
								isInShadow = true;
							}
						}
					}

					for (auto& plane : pScene->GetPlaneGeometries())
					{
						Vector3 normal = plane.origin - closestHit.origin;
						normal.Normalize();
						//move closestHit.origin along the normal by 0.1f and put into movedHit
						//to avoid self-shadowing
						Vector3 movedHit{};
						Ray lightRay = { movedHit, distanceToLightVector, 0.001f, distanceToLight };
						if (pScene->DoesHit(lightRay))
						{
							//occluder between light and plane
							if (lightRay.max < distanceToLight)
							{
								//shadow
							}
							else if (lightRay.max > distanceToLight || lightRay.max == distanceToLight)
							{
								//lit
							}
						}
					}

				}
				if (isInShadow)
					finalColor = materials[closestHit.materialIndex]->Shade();
				else
					finalColor = (materials[closestHit.materialIndex]->Shade()) * 0.5f;
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
