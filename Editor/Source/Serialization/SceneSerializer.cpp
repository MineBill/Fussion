#include "SceneSerializer.h"
#include "EditorApplication.h"
#include "Helpers.h"

#include "Engin5/Scene/Scene.h"

#include "kdlpp.h"

using kdl::Node;
using kdl::Document;

void SerializeStruct(Node& root, Reflect::TypeInfo const& type_info)
{
    // @todo Figure out a way to collect all the public fields.
    for (auto const& info : type_info.GetMemberInfosWithFlag("Serialize")) {
        EASSERT(info.GetPropertyType() == Reflect::PropertyType::Member);

        if (info.IsPrimitiveType()) {
            if (info.IsType<u32>() || info.IsType<u64>()) {

            } else if (info.IsType<f32>() || info.IsType<f64>()) {

            } else if (info.IsType<bool>()) {

            }
        } else if (info.IsClassType()) {
            LOG_DEBUGF("CLASS TYPE: {}",  info.GetType().GetTypeName());
        }
    }
}

void SceneSerializer::Save(AssetMetadata metadata, Engin5::Asset* asset)
{
    auto scene = cast(Engin5::Scene*, asset);
    EASSERT(scene != nullptr);

    Node root{"Scene"};
    for (auto const& [handle, entity] : scene->m_Entities) {
        Node node{"Entity"};

        node.properties().insert_or_assign("Name", entity.m_Name);
        node.properties().insert_or_assign("Handle", cast(u64, handle));
        node.properties().insert_or_assign("Parent", cast(u64, entity.m_Parent));

        for (auto const& [id, component] : entity.m_Components) {
            auto type_info = component->GetTypeInfo();
            Node comp_node{type_info.GetType().GetPrettyTypeName()};

            SerializeStruct(comp_node, type_info);
        }

        root.children().push_back(node);
    }
}

Engin5::Asset* SceneSerializer::Load(AssetMetadata metadata)
{
    auto scene = new Engin5::Scene();
    auto path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    auto data = kdl::parse(path.string());

    if (auto root = FindNode(data.nodes(), "Scene")) {
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