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

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float cx= ((2 * (px + 0.5f) / screenWidth) - 1) * aspectRatio;
			float cy= 1 - (2 * (py + 0.5f) / screenHeight);

			Vector3 rayDirection{ cx,cy,camera.forward.z };
			rayDirection.Normalize();

			/*Ray hitRay{ {0,0,0},rayDirection };
			ColorRGB finalColor{ rayDirection.x,rayDirection.y, rayDirection.z };*/

			Ray viewRay{ {0,0,0}, rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};

			Sphere testSphere{ {0.f,0.f,100.f}, 50.f, 0 };

			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();
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
