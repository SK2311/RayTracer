#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float a{ Vector3::Dot(ray.direction, ray.direction) };
			float b{ (Vector3::Dot(2 * ray.direction, (ray.origin - sphere.origin))) };
			float c{ Vector3::Dot((ray.origin - sphere.origin), (ray.origin - sphere.origin)) - (sphere.radius * sphere.radius) };

			float discriminant{ (b*b) - (4 * a * c) };

			if (discriminant > 0.f)
			{
				float sqrtCalculation{ sqrt(discriminant) };
				float divider{ (2 * a) };

				float t{ (-b - sqrtCalculation) / divider };
				if (t < ray.min || t > ray.max)
				{
					t = (-b + sqrtCalculation) / divider;

					if (t < ray.min || t > ray.max)
					{
						return false;
					}
				}

				const Vector3 pointI1{ ray.origin + ray.direction * t };
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = pointI1;
				hitRecord.normal = (pointI1 - sphere.origin) / sphere.radius;
				return true;
			}			

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float t{ Vector3::Dot((plane.origin - ray.origin), plane.normal) };
			t /= Vector3::Dot(ray.direction, plane.normal);
			if (t > ray.min && t < ray.max)
			{
				const Vector3 pointI1{ ray.origin + ray.direction * t };
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.origin = pointI1;
				hitRecord.normal = plane.normal;
				return true;
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			//todo W5
			//assert(false && "No Implemented Yet!");
			
			//using 2 of the triangles vectors, we can get the normal vector
			/*Vector3 a { triangle.v1 - triangle.v0 };
			Vector3 b{ triangle.v2 - triangle.v0 };
			Vector3 normal{ Vector3::Cross(a,b) };*/

			//Vector3 normal{ triangle.normal };

			//switch (triangle.cullMode)
			//{
			//case TriangleCullMode::FrontFaceCulling:
			//	//check if we are hitting the front face
			//	if (!ignoreHitRecord)
			//	{
			//		if (Vector3::Dot(normal, ray.direction) < 0)
			//			return false;
			//	}
			//	else
			//	{
			//		if (Vector3::Dot(normal, ray.direction) > 0)
			//			return false;
			//	}
			//	break;
			//case TriangleCullMode::BackFaceCulling:
			//	//check if we are hitting the back face
			//	if (!ignoreHitRecord)
			//	{
			//		if (Vector3::Dot(normal, ray.direction) > 0)
			//			return false;
			//	}
			//	else
			//	{
			//		if (Vector3::Dot(normal, ray.direction) < 0)
			//			return false;
			//	}
			//	break;
			//case TriangleCullMode::NoCulling:
			//	//if the normal and the view direction are perpendicular, return false
			//	if (Vector3::Dot(normal, ray.direction) == 0)
			//		return false;
			//	break;
			//default:
			//	break;
			//}

			//Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3 };
			//Vector3 L{ center - ray.origin };
			//float t{ Vector3::Dot(L, normal) / Vector3::Dot(ray.direction, normal) };

			//if (t < ray.min || t > ray.max)
			//	return false;

			////hitpoint of ray on the plane
			//Vector3 p{ ray.origin + ray.direction * t };

			////get the other 2 edges of the triangle edgeA is the same as variable a, so no need to calculate it again
			//Vector3 edgeA{ triangle.v1 - triangle.v0 };
			//Vector3 edgeB{ triangle.v2 - triangle.v1 };
			//Vector3 edgeC{ triangle.v0 - triangle.v2 };

			////check if point is inside of the triangle by checking if the point is on the correct side of the triangle edges
			//Vector3 pointToSide{ p - triangle.v0 };
			//if (Vector3::Dot(normal, Vector3::Cross(edgeA, pointToSide)) < 0)
			//	return false;

			//pointToSide = p - triangle.v1;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeB, pointToSide)) < 0)
			//	return false;

			//pointToSide = p - triangle.v2;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeC, pointToSide)) < 0)
			//	return false;

			////if the point is in the trianlge, fill in the hitrecord and return true
			//hitRecord.didHit = true;
			//hitRecord.t = t;
			//hitRecord.materialIndex = triangle.materialIndex;
			//hitRecord.origin = p;
			//hitRecord.normal = triangle.normal;
			//return true;


			//Moller-Trumbore
			const float kEpsilon = 0.0000001f;

			Vector3 v0v1 = triangle.v1 - triangle.v0;
			Vector3 v0v2 = triangle.v2 - triangle.v0;
			Vector3 perVec = Vector3::Cross(ray.direction, v0v2);

			float dot = Vector3::Dot(v0v1, perVec);

			Vector3 normal{ triangle.normal };

			//check if the ray is parallel to the triangle, return false if it is
			if (dot < -kEpsilon && dot > kEpsilon)
				return false;

			float invDet = 1 / dot;

			Vector3 tVec = ray.origin - triangle.v0;
			float u = invDet * Vector3::Dot(tVec, perVec);
			if (u < 0.f || u > 1.f)
				return false;

			Vector3 qVec = Vector3::Cross(tVec, v0v1);
			float v = invDet * Vector3::Dot(ray.direction, qVec);
			if (v < 0.f || u + v > 1.f)
				return false;

			float t = invDet * Vector3::Dot(v0v2, qVec);
			if (t < ray.min || t > ray.max)
				return false;

			TriangleCullMode currentCullmode = triangle.cullMode;

			if (ignoreHitRecord)
			{
				switch (currentCullmode)
				{
				case dae::TriangleCullMode::FrontFaceCulling:
					currentCullmode = TriangleCullMode::BackFaceCulling;
					break;
				case dae::TriangleCullMode::BackFaceCulling:
					currentCullmode = TriangleCullMode::FrontFaceCulling;
					break;
				default:
					break;
				}
			}

			switch (currentCullmode)
			{
			case TriangleCullMode::FrontFaceCulling:
				//check if we are hitting the front face
				if (Vector3::Dot(normal, ray.direction) < 0)
					return false;
				break;
			case TriangleCullMode::BackFaceCulling:
				//check if we are hitting the back face
				if (Vector3::Dot(normal, ray.direction) > 0)
					return false;
				break;
			case TriangleCullMode::NoCulling:
				//if the normal and the view direction are perpendicular, return false
				if (Vector3::Dot(normal, ray.direction) == 0)
					return false;
				break;
			default:
				break;
			}

			if (t > 0)
			{
				Vector3 p{ ray.origin + ray.direction * t };

				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = p;
				hitRecord.normal = triangle.normal;
				return true;
			}
			
			return false;

		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region SlabTest TriangleMesh
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			//assert(false && "No Implemented Yet!");

			//slabtest
			if (!SlabTest_TriangleMesh(mesh, ray))
				return false;
			
			auto triangle = Triangle{};
			int nrOfTriangles{ (int)mesh.transformedNormals.size() };
			HitRecord closestHit{};

			for (int i{}; i < nrOfTriangles; ++i)
			{
				triangle.v0 = mesh.transformedPositions[mesh.indices[i * 3]];
				triangle.v1 = mesh.transformedPositions[mesh.indices[i * 3 + 1]];
				triangle.v2 = mesh.transformedPositions[mesh.indices[i * 3 + 2]];

				triangle.normal = mesh.transformedNormals[i];
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				if (HitTest_Triangle(triangle, ray, hitRecord, ignoreHitRecord))
				{
					if (closestHit.t > hitRecord.t)
						closestHit = hitRecord;
				}
			}

			hitRecord = closestHit;

			return closestHit.didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			Vector3 direction{};
			direction = light.origin - origin;
			return direction;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			//assert(false && "No Implemented Yet!");
			switch (light.type)
			{
			case LightType::Point:
			{
				const float intensity{ light.intensity };
				const float sphereRadiusSquared{ (light.origin - target).SqrMagnitude() };

				return light.color * (intensity / sphereRadiusSquared);
				break;
			}
			case LightType::Directional:
			{
				return light.color * light.intensity;
				break;
			}
			}
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}