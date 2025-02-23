#pragma once

#include "FileSystem/UID.h"
#include "Geometry/AABB.h"

#include "Components/ComponentAreaLight.h"
#include "Components/ComponentPointLight.h"
#include "Components/ComponentSpotLight.h"
#include "Components/ComponentLocalIBL.h"
#include "Components/ComponentPlanarReflection.h"

class Component;
class ComponentCamera;
class ComponentCanvas;
class ComponentParticleSystem;
class ComponentLine;

class GameObject;

class Quadtree;
class Cubemap;
class Updatable;

struct Bone;

enum class Premade3D
{
	CUBE,
	SPHERE,
	PLANE,
	CYLINDER,
	CAPSULE,
	CHARACTER
};

enum SearchingMeshes
{
	NON_REFLECTIVE = 0x00000001,
};

class Scene
{
public:
	Scene();
	~Scene();

	void FillQuadtree(const std::vector<GameObject*>& gameObjects);
	bool IsInsideACamera(const OBB& obb) const;
	bool IsInsideACamera(const AABB& aabb) const;

	std::vector<GameObject*> ObtainObjectsInFrustum(const math::Frustum* frustum, const int& filter = 0);
	std::vector<GameObject*> ObtainStaticObjectsInFrustum(const math::Frustum* frustum, const int& filter = 0);
	std::vector<GameObject*> ObtainDynamicObjectsInFrustum(const math::Frustum* frustum, const int& filter = 0);
	
	GameObject* CreateGameObject(const std::string& name, GameObject* parent, bool is3D = true);
	GameObject* DuplicateGameObject(const std::string& name, GameObject*, GameObject* parent);
	GameObject* CreateCameraGameObject(const std::string& name, GameObject* parent);
	GameObject* CreateCanvasGameObject(const std::string& name, GameObject* parent);
	GameObject* CreateUIGameObject(const std::string& name, GameObject* parent, ComponentType type);
	GameObject* Create3DGameObject(const std::string& name, GameObject* parent, Premade3D type);
	GameObject* CreateLightGameObject(const std::string& name,
									  GameObject* parent,
									  LightType type,
									  AreaType areaType = AreaType::NONE);
	GameObject* CreateAudioSourceGameObject(const char* name, GameObject* parent);
	void DestroyGameObject(const GameObject* gameObject);

	void RemoveComponentLineOfObject(const GameObject* gameObject);
	void RemoveEndOfLine(const GameObject* gameObject);

	void ConvertModelIntoGameObject(const std::string& model);

	GameObject* SearchGameObjectByID(UID gameObjectID) const;
	std::vector<GameObject*> SearchGameObjectByTag(const std::string& gameObjectTag) const;

	void RenderDirectionalLight() const;
	void RenderPointLights() const;
	void RenderSpotLights() const;
	void RenderAreaLights() const;
	void RenderAreaSpheres() const;
	void RenderAreaTubes() const;
	void RenderLocalIBLs() const;
	void RenderPlanarReflections() const;
	void RenderPointLight(const ComponentPointLight* compPoint);
	void RenderSpotLight(const ComponentSpotLight* compSpot);
	void RenderAreaSphere(const ComponentAreaLight* compSphere);
	void RenderAreaTube(const ComponentAreaLight* compTube);
	void RenderLocalIBL(const ComponentLocalIBL* compLocal) const;
	void RenderPlanarReflection(const ComponentPlanarReflection* compPlanar) const;

	void UpdateScenePointLights();
	void UpdateSceneSpotLights();
	void UpdateSceneAreaLights();
	void UpdateSceneMeshRenderers();
	void UpdateSceneBoundingBoxes();
	void UpdateSceneAgentComponents();
	void UpdateSceneAreaSpheres();
	void UpdateSceneAreaTubes();
	void UpdateSceneLocalIBLs();
	void UpdateScenePlanarReflections();
	void UpdateScenePointLight(const ComponentPointLight* compPoint);
	void UpdateSceneSpotLight(const ComponentSpotLight* compSpot);
	void UpdateSceneAreaSphere(const ComponentAreaLight* compSphere);
	void UpdateSceneAreaTube(const ComponentAreaLight* compTube);
	void UpdateSceneLocalIBL(ComponentLocalIBL* compLocal);
	void UpdateScenePlanarReflection(ComponentPlanarReflection* compPlanar);

	
	GameObject* GetRoot() const;
	Quadtree* GetRootQuadtree() const;
	Cubemap* GetCubemap() const;
	const size_t GetSizeSpotLights() const;
	const size_t GetSizePointLights() const;
	const int GetPointIndex(const ComponentPointLight* point);
	const int GetSphereIndex(const ComponentAreaLight* sphere);
	const int GetSpotIndex(const ComponentSpotLight* spot);
	const int GetTubeIndex(const ComponentAreaLight* tube);
	const GameObject* GetDirectionalLight() const;
	SpotLight GetSpotLightsStruct(int index) const;
	PointLight GetPointLightsStruct(int index) const;
	const std::vector<GameObject*>& GetNonStaticObjects() const;
	const std::vector<GameObject*>& GetSceneGameObjects() const;
	const std::vector<ComponentCamera*>& GetSceneCameras() const;
	const std::vector<ComponentCanvas*>& GetSceneCanvas() const;
	const std::vector<Component*>& GetSceneInteractable() const;
	const std::vector<Updatable*>& GetSceneUpdatable() const;
	const std::vector<ComponentVideo*>& GetSceneVideos() const;
	const std::vector<ComponentParticleSystem*>& GetSceneParticleSystems() const;
	const std::vector<ComponentLine*>& GetSceneComponentLines() const;
	const std::vector<ComponentPlanarReflection*>& GetScenePlanarReflections() const;
	std::vector<AABB> GetBoundingBoxes() const;
	std::vector<ComponentAgent*> GetAgentComponents() const;
	std::vector<float> GetVertices();
	std::vector<int> GetTriangles();
	std::vector<float> GetNormals();
	std::unique_ptr<Quadtree> GiveOwnershipOfQuadtree();
	std::unordered_map<const ComponentPointLight*, unsigned int>& GetCachedPointLights();
	std::unordered_map<const ComponentSpotLight*, unsigned int>& GetCachedSpotLights();
	std::unordered_map<const ComponentAreaLight*, unsigned int>& GetCachedSphereLights();
	std::unordered_map<const ComponentAreaLight*, unsigned int>& GetCachedTubeLights();

	void SetRoot(GameObject* newRoot);
	void SetRootQuadtree(std::unique_ptr<Quadtree> quadtree);
	void SetCubemap(std::unique_ptr<Cubemap> cubemap);
	void SetSceneGameObjects(const std::vector<GameObject*>& gameObjects);
	void SetSceneCameras(const std::vector<ComponentCamera*>& cameras);
	void SetSceneCanvas(const std::vector<ComponentCanvas*>& canvas);
	void SetSceneInteractable(const std::vector<Component*>& interactable);
	void SetDirectionalLight(GameObject* directionalLight);

	void AddSceneGameObjects(const std::vector<GameObject*>& gameObjects);
	void AddSceneCameras(const std::vector<ComponentCamera*>& cameras);
	void AddSceneCanvas(const std::vector<ComponentCanvas*>& canvas);
	void AddSceneInteractable(const std::vector<Component*>& interactable);
	void AddSceneParticleSystem(const std::vector<ComponentParticleSystem*>& particleSystems);

	void AddStaticObject(GameObject* gameObject);
	void RemoveStaticObject(const GameObject* gameObject);
	void AddNonStaticObject(GameObject* gameObject);
	void RemoveNonStaticObject(const GameObject* gameObject);
	void AddUpdatableObject(Updatable* updatable);
	
	void AddVideoComponent(ComponentVideo* componentVideo);
	void AddParticleSystem(ComponentParticleSystem* particleSystem);
	void AddComponentLines(ComponentLine* componentLine);
	void AddPlanarReflection(ComponentPlanarReflection* componentPlanarReflection, bool updateBuffer);
	
	void RemoveParticleSystem(const ComponentParticleSystem* particleSystem);
	void RemoveComponentLine(const ComponentLine* componentLine);
	void RemovePlanarReflection(const ComponentPlanarReflection* componentPlanarReflection);

	void InitNewEmptyScene();
	void InitLights();
	void InitRender();
	void InitCubemap();
	void InitLocalsIBL();

private:
	void InsertGameObjectAndChildrenIntoSceneGameObjects(GameObject* gameObject, bool is3D, int& filter);
	void UpdateLightsFromCopiedGameObjects(const int& filter);

	enum LightsFilter
	{
		HAS_SPOT = 0x00000001,
		HAS_POINT = 0x00000002,
		HAS_AREA_TUBE = 0x00000004,
		HAS_AREA_SPHERE = 0X00000008,
		HAS_LOCAL_IBL = 0X00000010,
		HAS_PLANAR_REFLECTION = 0X00000020
	};

	int SearchForLights(GameObject* gameObject);

	GameObject* FindRootBone(GameObject* node, const std::vector<Bone>& bones);
	const std::vector<GameObject*> CacheBoneHierarchy(GameObject* gameObjectNode, const std::vector<Bone>& bones);
	void RemoveFatherAndChildren(const GameObject* father);
	void GenerateLights();
	void RemoveGameObjectFromScripts(const GameObject* gameObject);

	void CalculateObjectsInFrustum(const math::Frustum* frustum, const Quadtree* quad,
		std::vector<GameObject*>& gos, const int& filter = 0);
	void CalculateNonStaticObjectsInFrustum(const math::Frustum* frustum, GameObject* go,
		std::vector<GameObject*>& gos, const int& filter = 0);
	bool FrustumInQuadTree(const math::Frustum* frustum, const Quadtree* quad);
	bool ObjectInFrustum(const math::Frustum* frustum, const AABB& aabb);

	std::unique_ptr<Cubemap> cubemap;
	std::unique_ptr<GameObject> root;

	std::vector<GameObject*> sceneGameObjects;
	std::vector<ComponentCamera*> sceneCameras;
	std::vector<ComponentCanvas*> sceneCanvas;
	std::vector<ComponentVideo*> sceneVideos;
	std::vector<Component*> sceneInteractableComponents;
	std::vector<Updatable*> sceneUpdatableObjects;

	// Draw is const so I need this vector
	std::vector<ComponentParticleSystem*> sceneParticleSystems;
	std::vector<ComponentLine*> sceneComponentLines;
	std::vector<ComponentPlanarReflection*> sceneComponentPlanarReflection;
	
	GameObject* directionalLight;
	GameObject* cubeMapGameObject;

	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
	std::vector<AreaLightSphere> sphereLights;
	std::vector<AreaLightTube> tubeLights;
	std::vector<LocalIBL> localIBLs;
	std::vector<PlanarReflection> planarReflections;
	
	std::vector<ComponentMeshRenderer*> meshRenderers;
	std::vector<AABB> boundingBoxes;
	std::vector<ComponentAgent*> agentComponents;

	std::unordered_map<const ComponentPointLight*, unsigned int> cachedPoints;
	std::unordered_map<const ComponentSpotLight*, unsigned int> cachedSpots;
	std::unordered_map<const ComponentAreaLight*, unsigned int> cachedSpheres;
	std::unordered_map<const ComponentAreaLight*, unsigned int> cachedTubes;
	std::vector<std::pair<const ComponentLocalIBL*, unsigned int>> cachedLocalIBLs;
	std::vector<std::pair<const ComponentPlanarReflection*, unsigned int>> cachedPlanarReflections;

	unsigned uboDirectional;
	unsigned ssboPoint;
	unsigned ssboSpot;
	unsigned ssboSphere;
	unsigned ssboTube;
	unsigned ssboLocalIBL;
	unsigned ssboPlanarReflection;

	AABB rootQuadtreeAABB;
	// Render Objects
	std::unique_ptr<Quadtree> rootQuadtree;
	std::vector<GameObject*> nonStaticObjects;
};

inline GameObject* Scene::GetRoot() const
{
	return root.get();
}

inline const GameObject* Scene::GetDirectionalLight() const
{
	return directionalLight;
}

inline const std::vector<GameObject*>& Scene::GetSceneGameObjects() const
{
	return sceneGameObjects;
}

inline void Scene::SetSceneGameObjects(const std::vector<GameObject*>& gameObjects)
{
	sceneGameObjects = gameObjects;
}

inline const std::vector<ComponentCamera*>& Scene::GetSceneCameras() const
{
	return sceneCameras;
}

inline const std::vector<ComponentCanvas*>& Scene::GetSceneCanvas() const
{
	return sceneCanvas;
}

inline const std::vector<Component*>& Scene::GetSceneInteractable() const
{
	return sceneInteractableComponents;
}

inline const std::vector<Updatable*>& Scene::GetSceneUpdatable() const
{
	return sceneUpdatableObjects;
}

inline const std::vector<ComponentParticleSystem*>& Scene::GetSceneParticleSystems() const
{
	return sceneParticleSystems;
}

inline const std::vector<ComponentVideo*>& Scene::GetSceneVideos() const
{
	return sceneVideos;
}


inline const std::vector<ComponentLine*>& Scene::GetSceneComponentLines() const
{
	return sceneComponentLines;
}

inline const std::vector<ComponentPlanarReflection*>& Scene::GetScenePlanarReflections() const
{
	return sceneComponentPlanarReflection;
}

inline void Scene::SetSceneCameras(const std::vector<ComponentCamera*>& cameras)
{
	sceneCameras = cameras;
}

inline void Scene::SetSceneCanvas(const std::vector<ComponentCanvas*>& canvas)
{
	sceneCanvas = canvas;
}

inline void Scene::SetSceneInteractable(const std::vector<Component*>& interactable)
{
	sceneInteractableComponents = interactable;
}

inline void Scene::SetDirectionalLight(GameObject* directionalLight)
{
	this->directionalLight = directionalLight;
}

inline Quadtree* Scene::GetRootQuadtree() const
{
	return rootQuadtree.get();
}

inline Cubemap* Scene::GetCubemap() const
{
	return cubemap.get();
}

inline std::vector<ComponentAgent*> Scene::GetAgentComponents() const
{
	return agentComponents;
}

inline std::vector<AABB> Scene::GetBoundingBoxes() const
{
	return boundingBoxes;
}

inline const std::vector<GameObject*>& Scene::GetNonStaticObjects() const
{
	return nonStaticObjects;
}

inline void Scene::AddNonStaticObject(GameObject* gameObject)
{
	nonStaticObjects.push_back(gameObject);
}

inline void Scene::AddParticleSystem(ComponentParticleSystem* particleSystem)
{
	sceneParticleSystems.push_back(particleSystem);
}

inline void Scene::AddComponentLines(ComponentLine* componentLine)
{
	sceneComponentLines.push_back(componentLine);
}

inline void Scene::AddPlanarReflection(ComponentPlanarReflection* componentPlanarReflection, bool updateBuffer)
{
	sceneComponentPlanarReflection.push_back(componentPlanarReflection);
	if (updateBuffer)
	{
		UpdateScenePlanarReflections();
		RenderPlanarReflections();
	}
}

inline void Scene::AddVideoComponent(ComponentVideo* componentVideo)
{
	sceneVideos.push_back(componentVideo);
}

inline void Scene::RemoveParticleSystem(const ComponentParticleSystem* particleSystem)
{
	if (this)
	{
		sceneParticleSystems.erase(std::remove_if(std::begin(sceneParticleSystems),
			std::end(sceneParticleSystems),
			[&particleSystem](ComponentParticleSystem* particle)
			{
				return particle == particleSystem;
			}),
			std::end(sceneParticleSystems));
	}
}

inline void Scene::RemoveComponentLine(const ComponentLine* componentLine)
{
	sceneComponentLines.erase(std::remove_if(std::begin(sceneComponentLines),
		std::end(sceneComponentLines),
		[&componentLine](ComponentLine* lines)
		{
			return lines == componentLine;
		}),
		std::end(sceneComponentLines));
}

inline void Scene::RemovePlanarReflection(const ComponentPlanarReflection* componentPlanarReflection)
{
	if (this)
	{
		sceneComponentPlanarReflection.erase(std::remove_if(std::begin(sceneComponentPlanarReflection),
			std::end(sceneComponentPlanarReflection),
			[&componentPlanarReflection](ComponentPlanarReflection* planar)
			{
				return planar == componentPlanarReflection;
			}),
			std::end(sceneComponentPlanarReflection));
		UpdateScenePlanarReflections();
		RenderPlanarReflections();
	}
}

inline const size_t Scene::GetSizeSpotLights() const
{
	return static_cast<int>(spotLights.size());
}

inline const size_t Scene::GetSizePointLights() const
{
	return static_cast<int>(pointLights.size());
}

inline std::unordered_map<const ComponentPointLight*, unsigned int>& Scene::GetCachedPointLights()
{
	return cachedPoints;
}

inline std::unordered_map<const ComponentSpotLight*, unsigned int>& Scene::GetCachedSpotLights()
{
	return cachedSpots;
}

inline std::unordered_map<const ComponentAreaLight*, unsigned int>& Scene::GetCachedSphereLights()
{
	return cachedSpheres;
}

inline std::unordered_map<const ComponentAreaLight*, unsigned int>& Scene::GetCachedTubeLights()
{
	return cachedTubes;
}

inline const int Scene::GetPointIndex(const ComponentPointLight* point)
{
	return cachedPoints[point];
}

inline const int Scene::GetSphereIndex(const ComponentAreaLight* sphere)
{
	return cachedSpheres[sphere];
}

inline const int Scene::GetSpotIndex(const ComponentSpotLight* spot)
{
	return cachedSpots[spot];
}

inline const int Scene::GetTubeIndex(const ComponentAreaLight* tube)
{
	return cachedTubes[tube];
}