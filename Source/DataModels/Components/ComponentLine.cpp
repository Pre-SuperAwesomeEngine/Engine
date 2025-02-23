#include "StdAfx.h"
#include "ComponentLine.h"

#include "Application.h"

#include "ModuleCamera.h"
#include "Camera/Camera.h"

#include "ModuleScene.h"
#include "Scene/Scene.h"

#include "FileSystem/Json.h"
#include "FileSystem/ModuleFileSystem.h"
#include "FileSystem/ModuleResources.h"
#include "Resources/ResourceTexture.h"

#include "ModuleProgram.h"
#include "Program/Program.h"

#include "Components/ComponentTransform.h"
#include <GL/glew.h>

ComponentLine::ComponentLine(const bool active, GameObject* owner) : Component(ComponentType::LINE, active, owner, true),
	numTiles(1),speed(0.0f),time(0.0f),dirtyBuffers(false),offset(float2::zero),tiling(float2::one), gradient(new ImGradient()),
	sizeFading(float2::one),sizeFadingPoints(float4::zero)
{
	App->ScheduleTask([this]()
		{
#include "FileSystem/ModuleResources.h"
			LoadBuffers();
			UpdateBuffers();
		});
}

ComponentLine::ComponentLine(const ComponentLine& other) :
	Component(other),numTiles(other.numTiles),speed(other.speed),time(other.time),dirtyBuffers(other.dirtyBuffers), 
	gradient(new ImGradient(other.gradient)),offset(other.offset),tiling(other.tiling),sizeFading(other.sizeFading),
	sizeFadingPoints(other.sizeFadingPoints), lineTexture(other.GetLineTexture())
{
	LoadBuffers();
	UpdateBuffers();
}

ComponentLine::~ComponentLine()
{
	glDeleteVertexArrays(1, &lineVAO);
	glDeleteBuffers(1,&lineEBO);
	glDeleteBuffers(1,&positionBuffers);
	glDeleteBuffers(1,&textureBuffers);
	glDeleteBuffers(1,&colorBuffers);
	delete gradient;
}

void ComponentLine::LoadBuffers()
{
	glGenVertexArrays(1, &lineVAO);
	glBindVertexArray(lineVAO);

	glGenBuffers(1, &lineEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);

	glGenBuffers(1, &positionBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffers);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &textureBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffers);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &colorBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffers);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float4) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	dirtyBuffers = true;
}

void ComponentLine::UpdateBuffers()
{
	if (dirtyBuffers)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, positionBuffers);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textureBuffers);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, colorBuffers);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float4) * (numTiles * 2 + 2), nullptr, GL_STATIC_DRAW);

		float step = 1.0f / float(numTiles);

		// VBO for position
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffers);
		float3* posData = reinterpret_cast<float3*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
		float lambda = 0.0f;
		float size = sizeFading[0];
		posData[0] = float3(0.0f, size * -0.5f, 0.0f);
		posData[1] = float3(0.0f, size * 0.5f, 0.0f);

		for (unsigned int i = 0; i < numTiles; ++i)
		{
			lambda = step * float(i + 1);
			size = sizeFading[0] + (sizeFading[1] - sizeFading[0]) * 
				ImGui::BezierValue(lambda, reinterpret_cast<float*>(&sizeFadingPoints));
			posData[i * 2 + 2 + 0] = float3(lambda, size * -0.5f, 0.0f);
			posData[i * 2 + 2 + 1] = float3(lambda, size * 0.5f, 0.0f);
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);

		// VBO for texture
		glBindBuffer(GL_ARRAY_BUFFER, textureBuffers);
		float2* texData = reinterpret_cast<float2*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
		texData[0] = float2(0.0f, 0.0f);
		texData[1] = float2(0.0f, 1.0f);

		for (unsigned int i = 0; i < numTiles; ++i)
		{
			texData[i * 2 + 2 + 0] = float2(step * float(i + 1), 0.0f);
			texData[i * 2 + 2 + 1] = float2(step * float(i + 1), 1.0f);
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);

		// VBO for color
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffers);
		float4* colorData = reinterpret_cast<float4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
		lambda = 0.0f;
		float stepsGradient = static_cast<float>(1) / static_cast<float>(numTiles);
		float color[4];
		gradient->getColorAt(0.0, color);
		colorData[0] = float4(color[0], color[1], color[2], color[3]);
		colorData[1] = float4(color[0], color[1], color[2], color[3]);
		for (unsigned int i = 0; i < numTiles; ++i)
		{
			gradient->getColorAt(stepsGradient * (i + 1), color);
			lambda = step * float(i + 1);
			colorData[i * 2 + 2 + 0] = float4(color[0], color[1], color[2], color[3]);
			colorData[i * 2 + 2 + 1] = float4(color[0], color[1], color[2], color[3]);
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);

		// EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
		unsigned int* indexPtr = reinterpret_cast<unsigned int*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
		indexPtr[0] = 0;
		indexPtr[1] = 1;

		for (unsigned int i = 0; i < numTiles; ++i)
		{
			indexPtr[i * 2 + 2 + 0] = i * 2 + 2;
			indexPtr[i * 2 + 2 + 1] = i * 2 + 3;
		}

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		dirtyBuffers = false;
	}
}

void ComponentLine::Render()
{
	ModuleCamera* camera = App->GetModule<ModuleCamera>();
	ComponentTransform* transform = GetOwner()->GetComponentInternal<ComponentTransform>();

	if (IsEnabled() && camera->GetCamera()->IsInside(transform->GetEncapsuledAABB()))
	{
#ifdef ENGINE
		//Draw the BoundingBox of ComponentLine
		if (transform->IsDrawBoundingBoxes() && App->GetPlayState() != Application::PlayState::RUNNING)
		{
			App->GetModule<ModuleDebugDraw>()->DrawBoundingBox(transform->GetObjectOBB());
		}
#endif //ENGINE

		Program* program = App->GetModule<ModuleProgram>()->GetProgram(ProgramType::COMPONENT_LINE);
		if (childGameObject != nullptr)
		{
			program->Activate();
			ModelMatrix(program);
			UpdateBuffers();

			program->BindUniformFloat2("offset", offset);
			program->BindUniformFloat2("tiling", tiling);
			program->BindUniformFloat("time", time);

			glActiveTexture(GL_TEXTURE0);
			if (lineTexture)
			{
				lineTexture->Load();
				glBindTexture(GL_TEXTURE_2D, lineTexture->GetGlTexture());
				program->BindUniformInt("hasTexture", 1);
			}
			else
			{
				program->BindUniformInt("hasTexture", 0);
			}

			glBindVertexArray(lineVAO);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 + 2 * numTiles);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
			program->Deactivate();
		}
	}
}

void ComponentLine::ModelMatrix(Program* program)
{
	if (childGameObject)
	{
		ModuleCamera* camera = App->GetModule<ModuleCamera>();
		const float4x4& view = camera->GetCamera()->GetViewMatrix();
		const float4x4& proj = camera->GetCamera()->GetProjectionMatrix();

		float3 globalPosition = GetOwner()->GetComponentInternal<ComponentTransform>()->GetGlobalPosition();
		float3 childGlobalPosition = childGameObject->GetComponentInternal<ComponentTransform>()->GetGlobalPosition();

		float3 middlePoint = (childGlobalPosition + globalPosition) / 2;
		float3 centerCamera = (camera->GetCamera()->GetPosition() - middlePoint).Normalized();
		float3 xAxis = childGlobalPosition - globalPosition;
		float3 normalized = (childGlobalPosition - globalPosition).Normalized();
		float3 yAxis = centerCamera.Cross(normalized);
		float3 zAxis = normalized.Cross(yAxis);

		const float4x4& model =
			float4x4(float4(xAxis, 0.0),
				float4(yAxis, 0.0),
				float4(zAxis, 0.0),
				GetOwner()->GetComponentInternal<ComponentTransform>()->GetGlobalMatrix().Col(3));

		program->BindUniformFloat4x4(0, proj.ptr(), true);
		program->BindUniformFloat4x4(1, view.ptr(), true);
		program->BindUniformFloat4x4(2, model.ptr(), true);
	}
}

void ComponentLine::InternalSave(Json& meta)
{
	meta["numTiles"] = static_cast<int>(numTiles);
	meta["speed"] = static_cast<float>(speed);
	meta["offset_x"] =  static_cast<float>(offset.x);
	meta["offset_y"] =  static_cast<float>(offset.y);
	meta["tiling_x"] =  static_cast<float>(tiling.x);
	meta["tiling_y"] =  static_cast<float>(tiling.y);
	meta["sizeFading_x"] = static_cast<float>(sizeFading.x);
	meta["sizeFading_y"] = static_cast<float>(sizeFading.y);
	meta["sizeFadingPoints_x"] = static_cast<float>(sizeFadingPoints.x);
	meta["sizeFadingPoints_y"] = static_cast<float>(sizeFadingPoints.y);
	meta["sizeFadingPoints_z"] = static_cast<float>(sizeFadingPoints.z);
	meta["sizeFadingPoints_w"] = static_cast<float>(sizeFadingPoints.w);
	meta["numberOfMarks"] = static_cast<int>( gradient->getMarks().size());
	std::list<ImGradientMark*> marks = gradient->getMarks();
	int i = 0;
	std::string value;
	Json jsonColors = meta["ColorsGradient"];
	for (ImGradientMark* const& mark : marks)
	{
		jsonColors[i]["color_x"] = static_cast<float>(mark->color[0]);
		jsonColors[i]["color_y"] = static_cast<float>(mark->color[1]);
		jsonColors[i]["color_z"] = static_cast<float>(mark->color[2]);
		jsonColors[i]["color_w"] = static_cast<float>(mark->color[3]);
		jsonColors[i]["pos"] =	   static_cast<float>(mark->position);
		i++;
	}
	UID uid = 0;
	std::string assetPath = "";
	if (lineTexture)
	{
		uid = lineTexture->GetUID();
		assetPath = lineTexture->GetAssetsPath();
	}

	meta["textureUID"] = static_cast<UID>(uid);
	meta["assetPathTexture"] = assetPath.c_str();

	if (childGameObject)
	{
		uid = childGameObject->GetUID();
	}

	meta["EndPoint"] = static_cast<UID>(uid);
}

void ComponentLine::InternalLoad(const Json& meta)
{
	numTiles = static_cast<int>(meta["numTiles"]);
	speed = static_cast<float>(meta["speed"]);
	offset.x = static_cast<float>(meta["offset_x"]);
	offset.y = static_cast<float>(meta["offset_y"]);
	tiling.x = static_cast<float>(meta["tiling_x"]);
	tiling.y = static_cast<float>(meta["tiling_y"]);
	sizeFading.y = static_cast<float>(meta["sizeFading_y"]);
	sizeFading.x = static_cast<float>(meta["sizeFading_x"]);
	sizeFadingPoints.x = static_cast<float>(meta["sizeFadingPoints_x"]);
	sizeFadingPoints.y = static_cast<float>(meta["sizeFadingPoints_y"]);
	sizeFadingPoints.z = static_cast<float>(meta["sizeFadingPoints_z"]);
	sizeFadingPoints.w = static_cast<float>(meta["sizeFadingPoints_w"]);
	int numberOfMarks = static_cast<int>(meta["numberOfMarks"]);
	gradient->getMarks().clear();

	Json jsonColors = meta["ColorsGradient"];

	for (int i = 0; i < numberOfMarks; ++i)
	{
		
		gradient->addMark(static_cast<float>(jsonColors[i]["pos"]), 
							ImColor(static_cast<float>(jsonColors[i]["color_x"]),
							static_cast<float>(jsonColors[i]["color_y"]),
							static_cast<float>(jsonColors[i]["color_z"]),
							static_cast<float>(jsonColors[i]["color_w"])));
	}
	gradient->refreshCache();


#ifdef ENGINE
	std::string path = meta["assetPathTexture"];
	bool materialExists = !path.empty() && App->GetModule<ModuleFileSystem>()->Exists(path.c_str());

	if (materialExists)
	{
		std::shared_ptr<ResourceTexture> lineTexture =
			App->GetModule<ModuleResources>()->RequestResource<ResourceTexture>(path);

		if (lineTexture)
		{
			SetLineTexture(lineTexture);
		}
	}
#else
	UID uidTexture = meta["textureUID"];
	std::shared_ptr<ResourceTexture> resourceTexture =
		App->GetModule<ModuleResources>()->SearchResource<ResourceTexture>(uidTexture);

	if (resourceTexture)
	{
		SetLineTexture(lineTexture);
	}
#endif // ENGINE

	UID endpointUID = meta["EndPoint"];

	if (endpointUID != 0)
	{
		UID newFieldUID;
		if (App->GetModule<ModuleScene>()->HasNewUID(endpointUID, newFieldUID))
		{
			childGameObject = 
				App->GetModule<ModuleScene>()->GetLoadedScene()->SearchGameObjectByID(newFieldUID);
		}
		else
		{
			childGameObject = 
				App->GetModule<ModuleScene>()->GetLoadedScene()->SearchGameObjectByID(endpointUID);
		}
	}

	LoadBuffers();
}

const GameObject* ComponentLine::GetEnd() const
{
	return childGameObject;
}

void ComponentLine::SetEnd(GameObject* end)
{
	childGameObject = end;
}
