#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);

		// temp
		m_Triangles.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		//todo W1 - Done -> spheres and planes

		HitRecord currentHitStats{};
		closestHit.t = ray.max;

		for (const Sphere& sphere : m_SphereGeometries)
		{
			GeometryUtils::HitTest_Sphere(sphere, ray, currentHitStats);
			if (currentHitStats.didHit && currentHitStats.t < closestHit.t)
				closestHit = currentHitStats;
		}

		for (const Plane& plane : m_PlaneGeometries)
		{
			GeometryUtils::HitTest_Plane(plane, ray, currentHitStats);
			if (currentHitStats.didHit && currentHitStats.t < closestHit.t)
				closestHit = currentHitStats;
		}

		//for (const Triangle& triangle : m_Triangles)
		//{
		//	GeometryUtils::HitTest_Triangle(triangle, ray, currentHitStats);
		//	if (currentHitStats.didHit && currentHitStats.t < closestHit.t)
		//		closestHit = currentHitStats;
		//}

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray, currentHitStats);
			if (currentHitStats.didHit && currentHitStats.t < closestHit.t)
				closestHit = currentHitStats;
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		//todo W3 - DONE
		HitRecord hitStats{};

		for (const Sphere& sphere : m_SphereGeometries)
		{
			if (GeometryUtils::HitTest_Sphere(sphere, ray, hitStats, true))
				return true;
		}

		for (const Plane& plane : m_PlaneGeometries)
		{
			if (GeometryUtils::HitTest_Plane(plane, ray, hitStats, true))
				return true;
		}

		//for (const Triangle& triangle : m_Triangles)
		//{
		//	if (GeometryUtils::HitTest_Triangle(triangle, ray, hitStats, true))
		//		return true;
		//}

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			if (GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray, hitStats, true))
				return true;
		}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion

#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//plane
		AddPlane({ -5.f,  0.f,  0.f }, {  1.f,  0.f,  0.f }, matId_Solid_Green);
		AddPlane({  5.f,  0.f,  0.f }, { -1.f,  0.f,  0.f }, matId_Solid_Green);
		AddPlane({  0.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, matId_Solid_Yellow);
		AddPlane({  0.f, 10.f,  0.f }, {  0.f, -1.f,  0.f }, matId_Solid_Yellow);
		AddPlane({  0.f,  0.f, 10.f }, {  0.f,  0.f, -1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({  0.f,   1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({  1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({  0.f,   3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({  1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		//light
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.1f));

		// temp
		//const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.f));
		//const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.f));
		//const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.f));
		//AddSphere({ -1.75f, 1.f, 0.f }, .75f, matLambertPhong1);
		//AddSphere({ 0.f,   1.f, 0.f }, .75f, matLambertPhong2);
		//AddSphere({ 1.75f, 1.f, 0.f }, .75f, matLambertPhong3);

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));

		//plane
		AddPlane({ 0.f,  0.f, 10.f }, { 0.f,  0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,  0.f,  0.f }, { 0.f,  1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f,  0.f }, { 0.f, -1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f,  0.f,  0.f }, {-1.f,  0.f,  0.f }, matLambert_GrayBlue);
		AddPlane({-5.f,  0.f,  0.f }, { 1.f,  0.f,  0.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({  0.f,   1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({  1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({  0.f,   3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({  1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//light
		AddPointLight({ 0.f,  5.f,   5.f }, 50.f,  ColorRGB{ 1.f,   0.61f, 0.45f });
		AddPointLight({-2.5f, 5.f,  -5.f }, 70.f,  ColorRGB{ 1.f,   0.8f,  0.45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f,  ColorRGB{ 0.34f, 0.47f, 0.68f });
	}

	void Scene_W3_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		const auto matLambert_Red = AddMaterial(new Material_Lambert(colors::Red, 1.f));
		const auto matLambertPhong_Blue = AddMaterial(new Material_LambertPhong(colors::Blue, 1.f, 1.f, 60.f));
		const auto matLambert_Yellow = AddMaterial(new Material_Lambert(colors::Yellow, 1.f));

		//plane
		AddPlane({ 0.f,  0.f, 0.f }, { 0.f,  1.f,  0.f }, matLambert_Yellow);

		//Spheres
		AddSphere({ -0.75f, 1.f, 0.f }, 1.f, matLambert_Red);
		AddSphere({  0.75f, 1.f, 0.f }, 1.f, matLambertPhong_Blue);

		//light
		AddPointLight({ 0.f,  5.f,   5.f }, 25.f, colors::White);
		AddPointLight({ 0.f,  2.5f, -5.f }, 25.f, colors::White);
	}
#pragma endregion

#pragma region Week4
	void Scene_W4_ReferenceScene::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.f, 0.1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//plane
		AddPlane({ 0.f,  0.f, 10.f }, { 0.f,  0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,  0.f,  0.f }, { 0.f,  1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f,  0.f }, { 0.f, -1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f,  0.f,  0.f }, {-1.f,  0.f,  0.f }, matLambert_GrayBlue);
		AddPlane({-5.f,  0.f,  0.f }, { 1.f,  0.f,  0.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere({-1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f,   1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({-1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f,   3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//CW Winding order
		const Triangle baseTriangle{ {-.75f, 1.5f, 0.f}, {0.75f, 0.f, 0.f}, {-0.75f, 0.f, 0.f} };

		m_pMeshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_pMeshes[0]->AppendTriangle(baseTriangle, true);
		m_pMeshes[0]->Translate({ -1.75f, 4.5f, 0.f });
		m_pMeshes[0]->UpdateAABB();
		m_pMeshes[0]->UpdateTransforms();

		m_pMeshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_pMeshes[1]->AppendTriangle(baseTriangle, true);
		m_pMeshes[1]->Translate({ 0.f, 4.5f, 0.f });
		m_pMeshes[1]->UpdateAABB();
		m_pMeshes[1]->UpdateTransforms();

		m_pMeshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_pMeshes[2]->AppendTriangle(baseTriangle, true);
		m_pMeshes[2]->Translate({ 1.75f, 4.5f, 0.f });
		m_pMeshes[2]->UpdateAABB();
		m_pMeshes[2]->UpdateTransforms();

		//light
		AddPointLight({ 0.f,  5.f,   5.f }, 50.f, ColorRGB{ 1.f,   0.61f, 0.45f });
		AddPointLight({-2.5f, 5.f,  -5.f }, 70.f, ColorRGB{ 1.f,   0.8f,  0.45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ 0.34f, 0.47f, 0.68f });
	}

	void Scene_W4_ReferenceScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const auto yawAngle{ (cosf(pTimer->GetTotal()) + 1.f) / 2.f * PI_2 };
		for (const auto pMesh : m_pMeshes)
		{
			pMesh->RotateY(yawAngle);
			pMesh->UpdateTransforms();
		}
	}

	void Scene_W4_test::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		//mat
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({0.49f, 0.57f, 0.57f}, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//plane
		AddPlane({ 0.f,  0.f, 10.f }, { 0.f,  0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,  0.f,  0.f }, { 0.f,  1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f,  0.f }, { 0.f, -1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f,  0.f,  0.f }, { -1.f,  0.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f,  0.f,  0.f }, { 1.f,  0.f,  0.f }, matLambert_GrayBlue);

		//temp triangle
		//auto triangle = Triangle{ {-0.75f, 0.5f, 0.f}, {-0.75f, 2.f, 0.f}, {0.75f, 0.5f, 0.f} };
		//triangle.cullMode = TriangleCullMode::NoCulling;
		//triangle.materialIndex = matLambert_White;
		//
		//m_Triangles.emplace_back(triangle);

		//Triangle Mesh
		m_pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		//m_pMesh->positions = {
		//	{-0.75f, -1.f, 0.f}, 
		//	{-0.75f, 1.f, 0.f}, 
		//	{0.75f, 1.f, 1.f}, 
		//	{0.75f, -1.f, 0.f} };
		//m_pMesh->indices =
		//{
		//	0, 1, 2,
		//	0, 2, 3
		//};
		//
		//m_pMesh->CalculateNormals();
		//
		//m_pMesh->Translate({ 0.f, 1.5f, 0.f });
		Utils::ParseOBJ("Resources/Table.obj",
			m_pMesh->positions,
			m_pMesh->normals,
			m_pMesh->indices);

		m_pMesh->Scale({ 0.05f, 0.05f, 0.05f });
		//m_pMesh->Translate({ 0.f, 1.f, 0.f });

		m_pMesh->UpdateAABB();
		m_pMesh->UpdateTransforms();

		//light
		AddPointLight({ 0.f,  5.f,   5.f }, 50.f, ColorRGB{ 1.f,   0.61f, 0.45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f,   0.8f,  0.45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ 0.34f, 0.47f, 0.68f });
	}
	void Scene_W4_test::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		if (!m_pMesh)
			return;
		m_pMesh->RotateY(PI_DIV_2 * pTimer->GetTotal());
		m_pMesh->UpdateTransforms();
	}

	void Scene_W4_BunnyScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		//mat
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//plane
		AddPlane({ 0.f,  0.f, 10.f }, { 0.f,  0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,  0.f,  0.f }, { 0.f,  1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f,  0.f }, { 0.f, -1.f,  0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f,  0.f,  0.f }, {-1.f,  0.f,  0.f }, matLambert_GrayBlue);
		AddPlane({-5.f,  0.f,  0.f }, { 1.f,  0.f,  0.f }, matLambert_GrayBlue);

		//Triangle Mesh
		m_pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny2.obj",
			m_pMesh->positions,
			m_pMesh->normals,
			m_pMesh->indices);

		m_pMesh->UpdateAABB();
		m_pMesh->UpdateTransforms();

		//light
		AddPointLight({ 0.f,  5.f,  5.f }, 50.f, ColorRGB{ 1.f,   0.61f, 0.45f });
		AddPointLight({-2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f,   0.8f,  0.45f });
		AddPointLight({ 2.5f, 2.5f,-5.f }, 50.f, ColorRGB{ 0.34f, 0.47f, 0.68f });
	}

#pragma endregion
}
