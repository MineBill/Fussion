#include "SceneSerializer.h"
#include "EditorApplication.h"
#include "Helpers.h"

#include "Fussion/Scene/Scene.h"

#include "kdlpp.h"
#include "Fussion/OS/FileSystem.h"

using kdl::Node;
using kdl::Document;

// void SerializeStruct(Node& root, Reflect::TypeInfo const& type_info)
// {
//     // @todo Figure out a way to collect all the public fields.
//     for (auto const& info : type_info.GetMemberInfosWithFlag("Serialize")) {
//         EASSERT(info.GetPropertyType() == Reflect::PropertyType::Member);
//
//         if (info.IsPrimitiveType()) {
//             if (info.IsType<u32>() || info.IsType<u64>()) {
//
//             } else if (info.IsType<f32>() || info.IsType<f64>()) {
//
//             } else if (info.IsType<bool>()) {
//
//             }
//         } else if (info.IsClassType()) {
//             LOG_DEBUGF("CLASS TYPE: {}",  info.GetType().GetTypeName());
//         }
//     }
// }

void SceneSerializer::Save(AssetMetadata metadata, Fussion::Asset* asset)
{
    auto scene = cast(Fussion::Scene*, asset);
    EASSERT(scene != nullptr);

    Node root{"Scene"};

    Node name{"Name"};
    name.args().emplace_back(scene->m_Name);
    root.children().push_back(name);
    for (auto const& [handle, entity] : scene->m_Entities) {
        Node node{"Entity"};

        node.properties().insert_or_assign("Name", entity.m_Name);
        node.properties().insert_or_assign("Handle", cast(u64, handle));
        node.properties().insert_or_assign("Parent", cast(u64, entity.m_Parent));

        for (auto const& [id, component] : entity.m_Components) {
            // auto type_info = component->GetTypeInfo();
            // Node comp_node{type_info.GetType().GetPrettyTypeName()};

            // SerializeStruct(comp_node, type_info);
        }

        root.children().push_back(node);
    }

    Document doc{{root}};

    auto full_path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    Fsn::FileSystem::WriteEntireFile(full_path, doc.to_string());
    LOG_DEBUGF("Saving scene {}", full_path.string());
}

Fussion::Asset* SceneSerializer::Load(AssetMetadata metadata)
{
    auto const scene = new Fussion::Scene();
    auto const path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    auto data = kdl::parse(path.string());

    if (auto root = FindNode(data.nodes(), "Scene")) {
        if (auto const n_name = FindNode(data.nodes(), "Name")) {
            scene->m_Name = *GetArgAt<std::string>(*n_name, 0, kdl::Type::String);
        }
        for (auto const& node : root->children()) {
            if (node.name() == "Entity") {
                auto handle = GetProperty<u64>(node, "Handle");
                auto name = GetProperty<std::string>(node, "Name");
                if (!handle.has_value() || !name.has_value()) continue;
                scene->CreateEntityWithID(*handle, *name);
            }
        }
    }
    return scene;
}