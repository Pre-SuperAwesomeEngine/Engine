#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

class ComponentParticleSystem;
class ComponentRigidBody;
class ComponentTransform;
class ComponentLight;
class SendTriggerCollision;
class ComponentAudioSource;

enum class BoostOfEnergyStates 
{
	AIMING,
	PRESHOOTING,
	SHOOTING,
	DEACTIVATING,
	RECHARGING,
	RECHARGED
};

class BoostOfEnergy : public Script
{
public:
	BoostOfEnergy();
	~BoostOfEnergy() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	void PerformAttack();
	bool IsAttackAvailable();
	bool IsAttacking();
	void InterruptAttack();

	void SetIsPaused(bool isPaused);

	BoostOfEnergyStates attackState;

private:
	void PreShootLaser();
	void ShootLaser();
	void DeactivateLaser();
	void RechargeLaser();

	void SetCollisionEnter(ComponentRigidBody* other);
	void SetCollisionExit(ComponentRigidBody* other);

	float cooldownDuration;
	float cooldownTimer;

	float aimingDuration;
	float aimingTimer;
	float preshootingDuration;
	float preshootingTimer;
	float shootingDuration;
	float shootingTimer;
	float deactivationDuration;
	float deactivationTimer;
	ComponentParticleSystem* aimingParticle;
	ComponentParticleSystem* preshootingParticle;
	ComponentParticleSystem* shootingParticle;
	ComponentParticleSystem* deactivatingParticle;

	float attackDamage;
	float damageFrequency;
	float lastDamageTime;
	float isPlayerInDamageZone;

	ComponentTransform* transform;
	ComponentRigidBody* damageZone;
	SendTriggerCollision* sendTriggerCollision;
	ComponentTransform* mesh;
	GameObject* target;
	ComponentLight* light;
	ComponentTransform* shootingPosition;
	ComponentAudioSource* audioSource;

	bool isPaused;
};