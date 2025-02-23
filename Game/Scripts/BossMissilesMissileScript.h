#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"
#include "Bullet\LinearMath\btVector3.h"

RUNTIME_MODIFIABLE_INCLUDE;

class ComponentAudioSource;
class ComponentParticleSystem;

class BossMissilesMissileScript : public Script
{
public:
	BossMissilesMissileScript();
	~BossMissilesMissileScript() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	void OnCollisionEnter(ComponentRigidBody* other) override;
	void SetIsPaused(bool isPaused);

private:
	void TriggerExplosion(float deltaTime);

	void DestroyMissile() const;

	ComponentRigidBody* rigidBody;
	ComponentAudioSource* audioSource;
	ComponentParticleSystem* areaEffectParticle;
	GameObject* explosionEffect;
	float hasHitPlayer;
	float hasHitGround;

	// Modifiable values
	float missileDamage;
	float explosionTime;
	float maxSizeExplosion;
	float areaGrowingFactor;

	bool isPaused;
	btVector3 missileGravity;
};