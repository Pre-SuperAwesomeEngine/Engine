#pragma once

#include "ParticleModule.h"

class ModulePosition : public ParticleModule
{
public:
	enum class ShapeType : int { CIRCLE = 0, CONE, BOX };

public:
	ModulePosition(ParticleEmitter* emitter);
	ModulePosition(ParticleEmitter* emitter, ModulePosition* position);
	~ModulePosition() override;

	void Spawn(EmitterInstance* instance) override;
	void Update(EmitterInstance* instance) override;
	void CopyConfig(ParticleModule* module) override {};

	void DrawImGui() override {};
};

