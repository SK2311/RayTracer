#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			//assert(false && "Not Implemented Yet");
			auto rho{ cd * kd };
			return rho / M_PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			//assert(false && "Not Implemented Yet");
			ColorRGB rho{ cd * kd };
			return rho / M_PI;
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//todo: W3
			//ALWAYS MAKE SURE THAT THE DOT PRODUCT IS LARGER THAN 0
			const auto r{ l - (2 * (std::max(0.f, Vector3::Dot(n, l)))) * n };
			const auto cosAlpha{ std::max(0.f, Vector3::Dot(r, v)) };
			const auto phong{ ks * (powf(cosAlpha, exp)) };
			ColorRGB phongRGB{ phong, phong, phong };
			return phongRGB;
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//todo: W3
			//ALWAYS MAKE SURE THAT THE DOT PRODUCT IS LARGER THAN 0
			const auto dot{ std::max(0.f, Vector3::Dot(h, v)) };
			const auto f{ f0 + (ColorRGB{1, 1, 1} - f0) * powf((1 - dot), 5) };
			return f;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			const auto alpha{ roughness * roughness };
			const auto alphaSquared{ alpha * alpha };
			//ALWAYS MAKE SURE THAT THE DOT PRODUCT IS LARGER THAN 0
			const auto dot{ std::max(0.f, Vector3::Dot(n, h)) };
			const auto d{ alphaSquared / (M_PI * Square((Square(dot) * (alphaSquared - 1) + 1))) };
			return d;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//todo: W3
			//ALWAYS MAKE SURE THAT THE DOT PRODUCT IS LARGER THAN 0
			const auto dot{ std::max(0.f, Vector3::Dot(n, v)) };
			const auto alpha{ roughness * roughness };	
			const auto k{ Square(alpha + 1) / 8 };
			const auto g{ dot / (dot * (1 - k) + k) };
			return g;
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			//todo: W3
			//assert(false && "Not Implemented Yet");
			return GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
		}
	}
}