#pragma once
#include "Auxiliar/Generics/Drawable.h"
#include "Component.h"

#include "GameObject/GameObject.h"

enum class LightType 
{ 
	UNKNOWN, 
	DIRECTIONAL, 
	POINT, 
	SPOT,
	AREA,
	LOCAL_IBL,
	PLANAR_REFLECTION
};

enum class AreaType
{
	SPHERE,
	TUBE,
	QUAD,
	DISK,
	NONE
};

const static std::string GetNameByLightType(LightType type);
const static LightType GetLightTypeByName(const std::string& name);

class Json;

class ComponentLight : public Component, public Drawable
{
public:
	ComponentLight(const bool active, GameObject* owner);
	ComponentLight(LightType type, bool canBeRemoved);
	ComponentLight(LightType type, GameObject* gameObject, bool canBeRemoved);
	ComponentLight(LightType type, const float3& color, float intensity, bool canBeRemoved);
	ComponentLight(LightType type, const float3& color, float intensity, GameObject* gameObject, bool canBeRemoved);
	ComponentLight(const ComponentLight& componentLight);

	virtual ~ComponentLight() override;

	virtual void Draw() const override{};

	virtual void InternalSave(Json& meta) override{};
	virtual void InternalLoad(const Json& meta) override{};

	virtual void OnTransformChanged() override{};

	const float3& GetColor() const;
	float GetIntensity() const;
	LightType GetLightType() const;
	const bool IsDeleting() const;
	const bool IsDirty() const;

	void SetColor(const float3& color);
	void SetIntensity(float intensity);
	void SetDirty(bool dirty);

protected:
	float3 color;
	float intensity;
	
	bool deleting;
	bool isDirty;

	LightType lightType;
};

inline const float3& ComponentLight::GetColor() const
{
	return color;
}

inline float ComponentLight::GetIntensity() const
{
	return intensity;
}

inline LightType ComponentLight::GetLightType() const
{
	return lightType;
}

inline const bool ComponentLight::IsDirty() const
{
	return isDirty;
}

inline void ComponentLight::SetColor(const float3& color)
{
	this->color = color;
}

inline void ComponentLight::SetIntensity(float intensity)
{
	this->intensity = intensity;
}

inline const bool ComponentLight::IsDeleting() const
{
	return deleting;
}

inline void ComponentLight::SetDirty(bool dirty)
{
	isDirty = dirty;
}

inline const std::string GetNameByLightType(LightType type)
{
	switch (type)
	{
		case LightType::DIRECTIONAL:
			return "LightType_Directional";
		case LightType::POINT:
			return "LightType_Point";
		case LightType::SPOT:
			return "LightType_Spot";
		case LightType::AREA:
			return "LightType_Area";
		case LightType::LOCAL_IBL:
			return "LightType_Local_IBL";
		case LightType::PLANAR_REFLECTION:
			return "LightType_Planar_Reflection";
		default:
			assert(false && "Wrong light type introduced");
			return std::string();
	}
}

inline const LightType GetLightTypeByName(const std::string& typeName)
{
	if (typeName == "LightType_Directional")
	{
		return LightType::DIRECTIONAL;
	}

	if (typeName == "LightType_Point")
	{
		return LightType::POINT;
	}

	if (typeName == "LightType_Spot")
	{
		return LightType::SPOT;
	}

	if (typeName == "LightType_Area")
	{
		return LightType::AREA;
	}

	if (typeName == "LightType_Local_IBL")
	{
		return LightType::LOCAL_IBL;
	}

	if (typeName == "LightType_Planar_Reflection")
	{
		return LightType::PLANAR_REFLECTION;
	}
	return LightType::UNKNOWN;
}