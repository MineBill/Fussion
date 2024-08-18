#include "SceneSerializer.h"

#include "EditorApplication.h"
#include "EditorPCH.h"

#include <Fussion/OS/FileSystem.h>
#include <Fussion/Scene/Scene.h>
#include <Fussion/Scene/Components/ScriptComponent.h>
#include <Fussion/Serialization/Json.h>
#include <Fussion/meta.hpp/meta_all.hpp>

using namespace Fussion;

void SceneSerializer::Save(EditorAssetMetadata metadata, Ref<Asset> const& asset)
{
    auto scene = asset->As<Scene>();
    VERIFY(scene != nullptr);

    ordered_json j = {
        { "$Type", "Scene" },
        { "Name", scene->m_Name },
    };

    j["Entities"] = json::array();

    u32 i = 0;
    for (auto const& [handle, entity] : scene->m_Entities) {
        if (handle == 0)
            continue;

        json en = {
            { "Name", entity.Name },
            { "Handle", entity.m_Handle },
            { "Parent", entity.m_Parent },
            { "Enabled", entity.m_Enabled },
            { "Transform", {
                  { "Position", ToJson(entity.Transform.Position) },
                  { "Rotation", ToJson(entity.Transform.EulerAngles) },
                  { "Scale", ToJson(entity.Transform.Scale) },
              } }
        };

        for (auto const& component : entity.GetComponents() | std::views::values) {
            auto ptr = component->meta_poly_ptr();
            auto component_type = ptr.get_type().as_pointer().get_data_type().as_class();
            auto name = component_type.get_metadata().at("Name").as<std::string>();

            en["Components"][name] = SerializeNativeClass(component_type, std::move(ptr));
        }

        j["Entities"][i++] = en;
    }

    auto full_path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    FileSystem::WriteEntireFile(full_path, j.dump(2));
}

Ref<Asset> SceneSerializer::Load(EditorAssetMetadata metadata)
{
    auto const scene = MakeRef<Scene>();
    auto const path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    auto const text = FileSystem::ReadEntireFile(path);

    struct ParentChildPair {
        Uuid Parent{}, Child{};
    };
    std::vector<ParentChildPair> entities_to_resolve{};

    try {
        auto j = json::parse(*text, nullptr, true, true);
        scene->m_Name = j["Name"].get<std::string>();

        auto entities = j["Entities"];
        VERIFY(entities.is_array());
        for (auto const& jentity : entities) {
            if (!jentity.contains("Name") || !jentity.contains("Handle"))
                continue;

            auto name = jentity["Name"].get<std::string>();
            auto handle = jentity["Handle"].get<Uuid>();
            auto parent = jentity["Parent"].get<Uuid>();
            auto enabled = jentity.value("Enabled", true);

            if (!parent.IsValid() && !scene->HasEntity(parent)) {
                entities_to_resolve.emplace_back(parent, handle);
            }

            auto* e = scene->CreateEntityWithId(handle, name, scene->HasEntity(parent) ? parent : Uuid(0));
            e->SetEnabled(enabled);

            if (jentity.contains("Transform")) {
                if (jentity["Transform"].contains("Position"))
                    e->Transform.Position = Vec3FromJson(jentity["Transform"]["Position"]);
                if (jentity["Transform"].contains("Rotation"))
                    e->Transform.EulerAngles = Vec3FromJson(jentity["Transform"]["Rotation"]);
                if (jentity["Transform"].contains("Scale"))
                    e->Transform.Scale = Vec3FromJson(jentity["Transform"]["Scale"]);
            }

            if (jentity.contains("Components")) {
                auto components_info = meta_hpp::resolve_scope("Components");
                for (auto const& [comp_name, comp_val] : jentity["Components"].items()) {
                    (void)comp_val;
                    if (auto info = components_info.get_typedef(comp_name); info.is_valid()) {
                        auto component = e->AddComponent(info.as_class());

                        auto ptr = component->meta_poly_ptr();
                        auto component_type = ptr.get_type().as_pointer().get_data_type().as_class();

                        DeserializeClassFromJson(comp_val, component_type, std::move(ptr));
                    }
                }
            }
        }

    } catch (std::exception const& e) {
        LOG_ERRORF("Exception while loading scene: {}", e.what());
    }

    for (auto const& [parent, child] : entities_to_resolve) {
        scene->GetEntity(child)->SetParent(*scene->GetEntity(parent));
    }
    return scene;
}
