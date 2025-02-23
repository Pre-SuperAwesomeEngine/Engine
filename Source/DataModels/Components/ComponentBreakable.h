#pragma once
#include "Auxiliar/Generics/Updatable.h"
#include "Bullet/LinearMath/btVector3.h"
#include "Components/Component.h"

#include "MathGeoLib/Include/Algorithm/Random/LCG.h"

class ComponentBreakable : public Component, public Updatable
{
public:
	ComponentBreakable(bool active, GameObject* owner);
	~ComponentBreakable() override;

	void Update() override;

	float GetImpulsionMul();
	void SetImpulsionMul(float impulsion);
	void SetSubscribed(bool subscribe);

	void BreakComponentBy(class ComponentRigidBody* other);
	void BreakComponent();
	void BreakComponentFalling();
	bool GetSubscribed();

private:
	void InternalSave(Json& meta) override;
	void InternalLoad(const Json& meta) override;

private:
	float impulsionForce = 1.0f;
	LCG lcg;
	btVector3 impulsion{ 1.0f, 1.0f, 1.0f };
	bool subscribed = true;
};

inline bool ComponentBreakable::GetSubscribed()
{
	return subscribed;
}

inline float ComponentBreakable::GetImpulsionMul()
{
	return impulsionForce;
}

inline void ComponentBreakable::SetImpulsionMul(float impulsion)
{
	impulsionForce = impulsion;
}

inline void ComponentBreakable::SetSubscribed(bool subscribe)
{
	subscribed = subscribe;
}
