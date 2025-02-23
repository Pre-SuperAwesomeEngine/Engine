#include "StdAfx.h"

#include "Scripting/Script.h"

#include "DataModels/Components/ComponentScript.h"

#include "Exceptions/ScriptAssertFailedException.h"

#include "Application.h"

Script::Script() : App(::App.get())
{
}

void Script::Enable()
{
	container->Enable();
}

void Script::Disable()
{
	container->Disable();
}

void Script::Serialize(ISimpleSerializer* pSerializer)
{
	SERIALIZE(owner);
	SERIALIZE(App);
	for (const TypeFieldPair& enumAndField : members)
	{
		switch (enumAndField.first)
		{
			case FieldType::FLOAT:
			{
				Field<float> field = std::get<Field<float>>(enumAndField.second);
				float value = field.getter();
				pSerializer->SerializeProperty(field.name.c_str(), value);
				field.setter(value);
				break;
			}

			case FieldType::FLOAT3:
			{
				Field<float3> field = std::get<Field<float3>>(enumAndField.second);
				float3 value = field.getter();
				pSerializer->SerializeProperty(field.name.c_str(), value);
				field.setter(value);
				break;
			}

			case FieldType::VECTOR:
			{
				VectorField field = std::get<VectorField>(enumAndField.second);
				std::vector<std::any> value = field.getter();
				pSerializer->SerializeProperty(field.name.c_str(), value);
				field.setter(value);
				break;
			}

			case FieldType::STRING:
			{
				Field<std::string> field = std::get<Field<std::string>>(enumAndField.second);
				std::string value = field.getter();
				pSerializer->SerializeProperty(field.name.c_str(), value);
				field.setter(value);
				break;
			}

			case FieldType::GAMEOBJECT:
			{
				Field<GameObject*> field = std::get<Field<GameObject*>>(enumAndField.second);
				GameObject* value = field.getter();
				pSerializer->SerializeProperty(field.name.c_str(), value);
				field.setter(value);
				break;
			}

			case FieldType::BOOLEAN:
			{
				Field<bool> field = std::get<Field<bool>>(enumAndField.second);
				bool value = field.getter();
				pSerializer->SerializeProperty(field.name.c_str(), value);
				field.setter(value);
				break;
			}

			case FieldType::STATEMACHINE:
			{
				Field<StateMachine*> field = std::get<Field<StateMachine*>>(enumAndField.second);
				StateMachine* value = field.getter();
				pSerializer->SerializeProperty(field.name.c_str(), value);
				field.setter(value);
				break;
			}

			default:
				break;
		}
	}
}

void Script::AddMember(TypeFieldPair&& member)
{
	members.push_back(member);
}

void Script::Assert(bool&& condition, std::string&& errorMessage)
{
	if (!condition)
	{
		LOG_ERROR(errorMessage);
		throw ScriptAssertFailedException("Assert failed during execution of script " +
										  std::string(GetConstructor()->GetName()));
	}
}
