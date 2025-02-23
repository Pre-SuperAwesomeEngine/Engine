#include "StdAfx.h"

#include "Application.h"
#include "ComponentRigidBody.h"
#include "ComponentTransform.h"
#include "FileSystem/Json.h"
#include "GameObject/GameObject.h"
#include "Geometry/Sphere.h"
#include "ModulePhysics.h"
#include "debugdraw.h"

#include "ComponentScript.h"

ComponentRigidBody::ComponentRigidBody(bool active, GameObject* owner) :
	Component(ComponentType::RIGIDBODY, active, owner, true)
{
	id = GenerateId();

	btTransform startTransform;
	startTransform.setIdentity();
	transform = GetOwner()->GetComponentInternal<ComponentTransform>();
	boxSize = transform->GetLocalAABB().HalfSize().Mul(transform->GetLocalScale());
	radius = transform->GetLocalAABB().MinimalEnclosingSphere().Diameter();
	factor = 0.5f;
	// WIP set proper default value
	height = 2.0f;

	currentShape = Shape::BOX;
	motionState = std::make_unique<btDefaultMotionState>(startTransform);
	shape = std::make_unique<btBoxShape>(btVector3{ boxSize.x, boxSize.y, boxSize.z });

	btRigidBody::btRigidBodyConstructionInfo inforb(100.f, motionState.get(), shape.get());
	inforb.m_friction = 0.0f;
	rigidBody = std::make_unique<btRigidBody>(inforb);
	SetUpMobility();

	rigidBody->setUserPointer(this); // Set this component as the rigidbody's user pointer
#ifdef ENGINE
	rigidBody->setCollisionFlags(btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
#endif // ENGINE

	SetLinearDamping(linearDamping);
	SetAngularDamping(angularDamping);

	SetCollisionShape(static_cast<ComponentRigidBody::Shape>(Shape::BOX));
}

ComponentRigidBody::ComponentRigidBody(const ComponentRigidBody& toCopy) :
	Component(toCopy),
	isKinematic(toCopy.isKinematic),
	isTrigger(toCopy.isTrigger),
	currentShape(toCopy.currentShape),
	translation(toCopy.translation),
	boxSize(toCopy.boxSize),
	radius(toCopy.radius),
	factor(toCopy.factor),
	height(toCopy.height),
	usePositionController(toCopy.usePositionController),
	useRotationController(toCopy.useRotationController),
	KpForce(toCopy.KpForce),
	KpTorque(toCopy.KpTorque),
	mass(toCopy.mass)
{
	// we need an owner to perform the calculations here, so use this hack to avoid having to change the existing code
	SetOwner(toCopy.GetOwner());

	id = GenerateId();

	transform = toCopy.transform;

	motionState = std::unique_ptr<btDefaultMotionState>(new btDefaultMotionState(*toCopy.motionState.get()));
	btRigidBody::btRigidBodyConstructionInfo inforb(toCopy.mass, motionState.get(), toCopy.shape.get());
	inforb.m_friction = 0.0f;
	rigidBody = std::make_unique<btRigidBody>(inforb);

	SetUpMobility();

	rigidBody->setUserPointer(this); // Set this component as the rigidbody's user pointer
#ifdef ENGINE
	rigidBody->setCollisionFlags(btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
#endif

	SetLinearDamping(toCopy.linearDamping);
	SetAngularDamping(toCopy.angularDamping);
	SetRestitution(toCopy.restitution);

	SetCollisionShape(currentShape);

	SetGravity(toCopy.gravity);

	// return the component to the expected state
	SetOwner(nullptr);
}

ComponentRigidBody::~ComponentRigidBody()
{
	App->GetModule<ModulePhysics>()->RemoveRigidBody(this, rigidBody.get());
}

void ComponentRigidBody::OnCollisionEnter(ComponentRigidBody* other)
{
	if (other == nullptr)
	{
		LOG_ERROR("Rigidbody owned by {} collided with a null game object; this should never happen.", GetOwner());
		return;
	}

	for (ComponentScript* script : GetOwner()->GetComponents<ComponentScript>())
	{
		script->OnCollisionEnter(other);
	}
}

void ComponentRigidBody::OnCollisionStay(ComponentRigidBody* other)
{
	AXO_TODO("Implement delegate for this")
	if (other == nullptr)
	{
		LOG_ERROR("Rigidbody owned by {} collided with a null game object; this should never happen.", GetOwner());
		return;
	}
}

void ComponentRigidBody::OnCollisionExit(ComponentRigidBody* other)
{
	if (other == nullptr)
	{
		LOG_ERROR("Rigidbody owned by {} collided with a null game object; this should never happen.", GetOwner());
		return;
	}

	for (ComponentScript* script : GetOwner()->GetComponents<ComponentScript>())
	{
		script->OnCollisionExit(other);
	}
}

void ComponentRigidBody::OnTransformChanged()
{
#ifdef ENGINE
	if (App->GetPlayState() == Application::PlayState::STOPPED)
	{
		UpdateRigidBody();
	}
#endif
}

void ComponentRigidBody::Draw() const
{

}

void ComponentRigidBody::Update()
{
	float deltaTime = App->GetDeltaTime();

	
	if (!rigidBody->isStaticOrKinematicObject())
	{
		rigidBody->setCcdMotionThreshold(0.1f);
		rigidBody->setCcdSweptSphereRadius(0.1f);

		btTransform trans;
		trans = rigidBody->getWorldTransform();
		btQuaternion rot = trans.getRotation();
		Quat currentRot = Quat(rot.x(), rot.y(), rot.z(), rot.w());
		transform->SetGlobalRotation(currentRot);
		btVector3 pos = rigidBody->getCenterOfMassTransform().getOrigin();
		float3 centerPoint = transform->GetLocalAABB().CenterPoint();
		btVector3 offset = trans.getBasis() * btVector3(centerPoint.x, centerPoint.y, centerPoint.z);
		float3 newPos = { pos.x(), pos.y(), pos.z()};
		newPos -= float3(translation.x(), translation.y(), translation.z());
		transform->SetGlobalPosition(newPos);
		transform->RecalculateLocalMatrix();
		transform->UpdateTransformMatrices();
	}

	if (usePositionController)
	{
		SimulatePositionController();
	}

	if (useRotationController)
	{
		SimulateRotationController();
	}
}

void ComponentRigidBody::SetOwner(GameObject* owner)
{
	Component::SetOwner(owner);
	if (owner != nullptr)
	{
		transform = GetOwner()->GetComponentInternal<ComponentTransform>();
	}
}

void ComponentRigidBody::UpdateRigidBody()
{
	btTransform worldTransform = rigidBody->getWorldTransform();
	float3 position = transform->GetGlobalPosition();
	btVector3 btPosition = btVector3(position.x, position.y, position.z);
	worldTransform.setOrigin(btPosition + translation);
	Quat rot = transform->GetGlobalRotation();
	worldTransform.setRotation({ rot.x, rot.y, rot.z, rot.w });
	rigidBody->setWorldTransform(worldTransform);
	motionState->setWorldTransform(worldTransform);
}

int ComponentRigidBody::GenerateId() const
{
	static uint32_t nextId = 1;

	assert(nextId != 0); // if this assert triggers, we have reached the maximum number of rigidbodies 2^32 - 1. This is
						 // a very unlikely scenario.
	return nextId++;
}

void ComponentRigidBody::SetRigidBodyOrigin(btVector3 origin)
{
	btTransform worldTransform = rigidBody->getWorldTransform();
	worldTransform.setOrigin(origin);
	rigidBody->setWorldTransform(worldTransform);
}

void ComponentRigidBody::UpdateRigidBodyTranslation()
{
	float3 position = transform->GetGlobalPosition();
	btVector3 btPosition = btVector3(position.x, position.y, position.z);

	translation = (rigidBody->getWorldTransform().getOrigin() - btPosition);
}

void ComponentRigidBody::SetUpMobility()
{
	RemoveRigidBodyFromDynamics();
	if (isKinematic)
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_DYNAMIC_OBJECT);
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT);
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidBody->setActivationState(DISABLE_DEACTIVATION);

		rigidBody->setMassProps(0, { 0, 0, 0 }); 
		AXO_TODO("Is this necessary here?");
		
	}
	else if (IsStatic())
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_DYNAMIC_OBJECT);
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
		rigidBody->setActivationState(DISABLE_DEACTIVATION);
		rigidBody->setMassProps(0, { 0, 0, 0 }); // static objects have no mass to avoid collision pushes
		isKinematic = false;
	}
	else
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT);
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_DYNAMIC_OBJECT);
		rigidBody->setActivationState(DISABLE_DEACTIVATION);
		btVector3 localInertia;
		rigidBody->getCollisionShape()->calculateLocalInertia(mass, localInertia);
		rigidBody->setMassProps(mass, localInertia);
	}
	AddRigidBodyToDynamics();
}

void ComponentRigidBody::SetCollisionShape(Shape newShape)
{
	switch (newShape)
	{
		case Shape::BOX: // Box
			shape = std::make_unique<btBoxShape>(btVector3{ boxSize.x, boxSize.y, boxSize.z });
			break;
		case Shape::SPHERE: // Sphere
			shape = std::make_unique<btSphereShape>(radius * factor);
			break;
		case Shape::CAPSULE: // Capsule
			shape = std::make_unique<btCapsuleShape>(radius, height);
			break;
		case Shape::CONE: // Cone
			shape = std::make_unique<btConeShape>(radius, height);
			break;

			/*case SHAPE::CYLINDER: // Cylinder
				shape = new btCylinderShape(btVector3(1, 1, 1));
				break;*/
	}

	if (shape)
	{
		currentShape = newShape;
		rigidBody->setCollisionShape(shape.get());
		// inertia for local rotation
		btVector3 localInertia;
		rigidBody->getCollisionShape()->calculateLocalInertia(mass, localInertia);
		rigidBody->setMassProps(mass, localInertia);
		rigidBody->updateInertiaTensor();
	}
}

void ComponentRigidBody::InternalSave(Json& meta)
{
	meta["isKinematic"] = static_cast<bool>(GetIsKinematic());
	meta["isTrigger"] = static_cast<bool>(IsTrigger());
	meta["drawCollider"] = static_cast<bool>(GetDrawCollider());
	meta["mass"] = static_cast<float>(GetMass());
	meta["linearDamping"] = static_cast<float>(GetLinearDamping());
	meta["angularDamping"] = static_cast<float>(GetAngularDamping());
	meta["restitution"] = static_cast<float>(GetRestitution());
	meta["currentShape"] = static_cast<int>(GetShape());
	meta["usePositionController"] = static_cast<bool>(GetUsePositionController());
	meta["useRotationController"] = static_cast<bool>(GetUseRotationController());
	meta["KpForce"] = static_cast<float>(GetKpForce());
	meta["KpTorque"] = static_cast<float>(GetKpTorque());
	meta["gravity_Y"] = static_cast<float>(GetGravity().getY());
	meta["boxSize_X"] = static_cast<float>(GetBoxSize().x);
	meta["boxSize_Y"] = static_cast<float>(GetBoxSize().y);
	meta["boxSize_Z"] = static_cast<float>(GetBoxSize().z);
	meta["radius"] = static_cast<float>(GetRadius());
	meta["factor"] = static_cast<float>(GetFactor());
	meta["height"] = static_cast<float>(GetHeight());
	meta["rbPos_X"] = static_cast<float>(GetRigidBodyOrigin().getX());
	meta["rbPos_Y"] = static_cast<float>(GetRigidBodyOrigin().getY());
	meta["rbPos_Z"] = static_cast<float>(GetRigidBodyOrigin().getZ());
	meta["xAxisBlocked"] = static_cast<bool>(IsXAxisBlocked());
	meta["yAxisBlocked"] = static_cast<bool>(IsYAxisBlocked());
	meta["zAxisBlocked"] = static_cast<bool>(IsZAxisBlocked());
	meta["xRotationAxisBlocked"] = static_cast<bool>(IsXRotationAxisBlocked());
	meta["yRotationAxisBlocked"] = static_cast<bool>(IsYRotationAxisBlocked());
	meta["zRotationAxisBlocked"] = static_cast<bool>(IsZRotationAxisBlocked());
}

void ComponentRigidBody::InternalLoad(const Json& meta)
{
	SetIsKinematic(static_cast<bool>(meta["isKinematic"]));
#ifdef ENGINE
	SetDrawCollider(static_cast<bool>(meta["drawCollider"]), false);
#endif
	SetIsTrigger(static_cast<bool>(meta["isTrigger"]));
	SetMass(static_cast<float>(meta["mass"]));
	SetLinearDamping(static_cast<float>(meta["linearDamping"]));
	SetAngularDamping(static_cast<float>(meta["angularDamping"]));

	SetRestitution(static_cast<float>(meta["restitution"]));
	SetUsePositionController(static_cast<bool>(meta["usePositionController"]));
	SetUseRotationController(static_cast<bool>(meta["useRotationController"]));
	SetKpForce(static_cast<float>(meta["KpForce"]));
	SetKpTorque(static_cast<float>(meta["KpTorque"]));
	SetBoxSize({ static_cast<float>(meta["boxSize_X"]),
				 static_cast<float>(meta["boxSize_Y"]),
				 static_cast<float>(meta["boxSize_Z"]) });
	SetRadius(static_cast<float>(meta["radius"]));
	SetFactor(static_cast<float>(meta["factor"]));
	SetHeight(static_cast<float>(meta["height"]));
	SetRigidBodyOrigin({ static_cast<float>(meta["rbPos_X"]),
						 static_cast<float>(meta["rbPos_Y"]),
						 static_cast<float>(meta["rbPos_Z"]) });

	int currentShape = static_cast<int>(meta["currentShape"]);

	if (currentShape != 0)
	{
		SetCollisionShape(static_cast<ComponentRigidBody::Shape>(currentShape));
	}

	SetUpMobility();
	SetGravity({ 0, static_cast<float>(meta["gravity_Y"]), 0 });

	SetXAxisBlocked(static_cast<bool>(meta["xAxisBlocked"]));
	SetYAxisBlocked(static_cast<bool>(meta["yAxisBlocked"]));
	SetZAxisBlocked(static_cast<bool>(meta["zAxisBlocked"]));

	SetXRotationAxisBlocked(static_cast<bool>(meta["xRotationAxisBlocked"]));
	SetYRotationAxisBlocked(static_cast<bool>(meta["yRotationAxisBlocked"]));
	SetZRotationAxisBlocked(static_cast<bool>(meta["zRotationAxisBlocked"]));
}

void ComponentRigidBody::SignalEnable()
{
	AddRigidBodyToDynamics();
}

void ComponentRigidBody::SignalDisable()
{
	RemoveRigidBodyFromDynamics();
}

void ComponentRigidBody::RemoveRigidBodyFromDynamics()
{
	App->GetModule<ModulePhysics>()->RemoveRigidBody(this, rigidBody.get());
}

void ComponentRigidBody::AddRigidBodyToDynamics()
{
	App->GetModule<ModulePhysics>()->AddRigidBody(this, rigidBody.get());
	rigidBody->setGravity(gravity);
}

void ComponentRigidBody::RemoveRigidBodyFromSimulation()
{
	isInCollisionWorld = false;
}

void ComponentRigidBody::AddRigidBodyToSimulation()
{
	isInCollisionWorld = true;
}

void ComponentRigidBody::ClearCollisionEnterDelegate()
{
	delegateCollisionEnter.clear();
}

void ComponentRigidBody::SetDrawCollider(bool newDrawCollider, bool substract)
{
	drawCollider = newDrawCollider;
	int value = 0;

	if (newDrawCollider)
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
		value = 1;
	}
	else
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

		if (substract)
		{
			value = -1;
		}
	}

	App->GetModule<ModulePhysics>()->UpdateDrawableRigidBodies(value);
}

void ComponentRigidBody::SetDefaultSize(Shape resetShape)
{
	switch (resetShape)
	{
		case Shape::BOX:
			boxSize = transform->GetLocalAABB().HalfSize().Mul(transform->GetLocalScale());
			break;
		case Shape::SPHERE:
			radius = transform->GetLocalAABB().MinimalEnclosingSphere().Diameter();
			factor = 0.5f;
			break;
		case Shape::CAPSULE:
			radius = transform->GetLocalAABB().MinimalEnclosingSphere().Diameter();
			height = 2.0f;
			break;
		case Shape::CONE:
			radius = transform->GetLocalAABB().MinimalEnclosingSphere().Diameter();
			height = 2.0f;
			break;
	}

	SetCollisionShape(resetShape);
	// WIP: reset 5th shape
}

void ComponentRigidBody::SetIsTrigger(bool newTrigger)
{
	isTrigger = newTrigger;
	if (newTrigger)
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CO_GHOST_OBJECT);
	}
	else
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CO_GHOST_OBJECT);
	}
}

void ComponentRigidBody::SetDefaultPosition()
{
	float3 transPos = transform->GetGlobalPosition();
	btVector3 transPosBt = btVector3(transPos.x, transPos.y, transPos.z);
	SetRigidBodyOrigin(transPosBt);
	UpdateRigidBodyTranslation();
	UpdateRigidBody();
}

bool ComponentRigidBody::IsStatic() const
{
	return GetOwner()->IsStatic();
}

void ComponentRigidBody::SetIsStatic(bool isStatic)
{
	GetOwner()->SetIsStatic(isStatic);
}

void ComponentRigidBody::UpdateBlockedAxis()
{
	rigidBody->setLinearFactor(btVector3(!IsXAxisBlocked(), !IsYAxisBlocked(), !IsZAxisBlocked()));
}

void ComponentRigidBody::UpdateBlockedRotationAxis()
{
	rigidBody->setAngularFactor(
		btVector3(!IsXRotationAxisBlocked(), !IsYRotationAxisBlocked(), !IsZRotationAxisBlocked()));
}

void ComponentRigidBody::SetAngularFactor(btVector3 rotation)
{
	rigidBody->setAngularFactor(btVector3(rotation.getX() * !IsXRotationAxisBlocked(),
										  rotation.getY() * !IsYRotationAxisBlocked(),
										  rotation.getZ() * !IsZRotationAxisBlocked()));
}

void ComponentRigidBody::SimulatePositionController()
{
	float3 x = transform->GetGlobalPosition();
	float3 positionError = targetPosition - x;
	float3 velocityPosition = positionError * KpForce;

	btVector3 velocity(velocityPosition.x * !IsXAxisBlocked(),
					   velocityPosition.y * !IsYAxisBlocked(),
					   velocityPosition.z * !IsZAxisBlocked());
	rigidBody->setLinearVelocity(velocity);
}

void ComponentRigidBody::SimulateRotationController()
{
	float3 axis;
	float angle;
	targetRotation.ToAxisAngle(axis, angle);
	axis.Normalize();

	float3 angularVelocity = axis * angle * KpTorque;
	btVector3 bulletAngularVelocity(0.0f, angularVelocity.y * !IsYRotationAxisBlocked(), 0.0f);
	rigidBody->setAngularVelocity(bulletAngularVelocity);
}
