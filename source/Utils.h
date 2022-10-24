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
			//todo W1 - DONE

			Vector3 sphereToRay{ ray.origin - sphere.origin };

			float a{ Vector3::Dot(ray.direction, ray.direction) };
			float b{Vector3::Dot(2.f * ray.direction, sphereToRay)};
			float c{Vector3::Dot(sphereToRay, sphereToRay) - Square(sphere.radius)};

			float discriminant{Square(b) - 4.f * a * c};

			if (discriminant > 0.f && IsInRange(discriminant, ray.min, ray.max))
			{
				hitRecord.t = (-b - sqrtf(discriminant)) / (2.f * a);
				if (!IsInRange(hitRecord.t, ray.min, ray.max))
				{
					hitRecord.t = (-b + sqrtf(discriminant)) / (2.f * a);
					if (!IsInRange(hitRecord.t, ray.min, ray.max))
						return false;
				}

				hitRecord.origin = ray.origin + hitRecord.t * ray.direction;
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			}
			else
			{
				hitRecord.didHit = false;
			}

			return hitRecord.didHit;
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
			//todo W1 - DONE
			
			Vector3 rayToPlane{plane.origin - ray.origin};
			float distance{Vector3::Dot(rayToPlane, plane.normal) / Vector3::Dot(ray.direction, plane.normal)};

			if (IsInRange(distance, ray.min, ray.max))
			{
				hitRecord.t = distance;
				hitRecord.origin = ray.origin + distance * ray.direction;
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.normal = plane.normal;
			}
			else
				hitRecord.didHit = false;


			return hitRecord.didHit;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TEST HELPER FUNCTIONS
		inline bool UseCulling(TriangleCullMode cullMode, float dotNV, bool ignoreHitRecord)
		{
			if (!ignoreHitRecord)
			{
				switch (cullMode)
				{
				case TriangleCullMode::BackFaceCulling:
					if (dotNV > 0.f)
						return true;
					break;
				case TriangleCullMode::FrontFaceCulling:
					if (dotNV < 0.f)
						return true;
					break;
				}
			}
			else
			{
				switch (cullMode)
				{
				case TriangleCullMode::BackFaceCulling:
					if (dotNV < 0.f)
						return true;
					break;
				case TriangleCullMode::FrontFaceCulling:
					if (dotNV > 0.f)
						return true;
					break;
				}
			}
			return false;
		}

		inline bool IsPointAtCorrectSide(Vector3 point, Vector3 v0, Vector3 v1, Vector3 normal)
		{
			Vector3 edge{ v1 - v0 };
			Vector3 pointToSide{ point - v0 };
			if (Vector3::Dot(normal, Vector3::Cross(edge, pointToSide)) < 0.f)
				return false;

			return true;
		}

		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			float dot{ Vector3::Dot(triangle.normal, ray.direction) };
			float epsilon{ 0.01f };

			if (dot > -epsilon && dot < epsilon)
			{
				return false;
			}

			if (UseCulling(triangle.cullMode, dot, ignoreHitRecord))
				return false;

			Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3.f };
			Vector3 l{ center - ray.origin };
			float distance{ Vector3::Dot(l, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal)};

			if (distance < ray.min || distance > ray.max)
				return false;

			Vector3 pointOnPlane{ ray.origin + ray.direction * distance };

			if (!IsPointAtCorrectSide(pointOnPlane, triangle.v0, triangle.v1, triangle.normal))
				return false;

			if (!IsPointAtCorrectSide(pointOnPlane, triangle.v1, triangle.v2, triangle.normal))
				return false;

			if (!IsPointAtCorrectSide(pointOnPlane, triangle.v2, triangle.v0, triangle.normal))
				return false;

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.normal = triangle.normal;
				hitRecord.origin = pointOnPlane;
				hitRecord.t = distance;
			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}

#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			const int triangleVertAmount{ 3 };
			float shortestDistance{ ray.max + 1.f };
			HitRecord currentRecord{};
			bool didHit{ false };
			int triangleCount{0};

			for (size_t i{}; (i + triangleVertAmount) <= mesh.indices.size(); i += triangleVertAmount)
			{
				uint32_t i0 = mesh.indices[i];
				uint32_t i1 = mesh.indices[i + 1];
				uint32_t i2 = mesh.indices[i + 2];

				Triangle triangle{ mesh.transformedPositions[i0], mesh.transformedPositions[i1], mesh.transformedPositions[i2], mesh.transformedNormals[triangleCount]};
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				if (HitTest_Triangle(triangle, ray, currentRecord, ignoreHitRecord))
				{
					if (currentRecord.t < shortestDistance)
					{
						hitRecord = currentRecord;
						shortestDistance = currentRecord.t;
						didHit = true;
					}
				}
				++triangleCount;
			}

			return didHit;
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
			return light.origin - origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3

			ColorRGB returnColor{};

			switch (light.type)
			{
			case LightType::Directional:
				break;

			case LightType::Point:
				float irradians{light.intensity / GetDirectionToLight(light, target).SqrMagnitude()};
				returnColor = light.color * irradians;
				break;
			}

			return returnColor;
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