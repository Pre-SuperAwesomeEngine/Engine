#pragma once
#include "FileSystem/Importers/Importer.h"
#include "DataModels/Resources/ResourceParticleSystem.h"

class ParticleModule;

class ParticleSystemImporter : public Importer<ResourceParticleSystem>
{
public:
    ParticleSystemImporter();
    ~ParticleSystemImporter() override;

    void Import(const char* filePath, std::shared_ptr<ResourceParticleSystem> resource) override;
	void Load(const char* fileBuffer, std::shared_ptr<ResourceParticleSystem> resource) override;
	void Save(const std::shared_ptr<ResourceParticleSystem>& resource, char*& fileBuffer, unsigned int& size) override;

private:
	void SaveModule(char*& cursor, ParticleModule*& module);
	void LoadModule(const char*& fileBuffer, ParticleModule*& module);
};
