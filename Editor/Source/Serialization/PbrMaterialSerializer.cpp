#include "PbrMaterialSerializer.h"
#include "Project/Project.h"

#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/Serialization/Json.h"

using namespace Fussion;

Ref<Asset> PbrMaterialSerializer::Load(AssetMetadata metadata)
{
    auto material = MakeRef<PbrMaterial>();

    auto const path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    auto const text = FileSystem::ReadEntireFile(path);

    try {
        auto j = json::parse(*text, nullptr, true, true);

        material->ObjectColor = j["Color"].get<Color>();
        material->Metallic = j.value("Metallic", 0.0f);
        material->Roughness = j.value("Roughness", 0.0f);
        material->AlbedoMap.SetHandle(j.value("Albedo", AssetHandle()));
    } catch (std::exception const& e) {
        LOG_ERRORF("Error while deserializing PbrMaterial: {}", e.what());
    }
    return material;
}

void PbrMaterialSerializer::Save(AssetMetadata metadata, Ref<Asset> const& asset)
{
    auto material = asset->As<PbrMaterial>();
    auto handle = material->AlbedoMap.Handle();
    if (material->AlbedoMap.IsVirtual()) {
        LOG_WARNF("Attempted serializing virtual asset '{}' for PbrMaterial::AlbedoMap", handle);
        handle = AssetHandle(0);
    }

    ordered_json j = {
        { "$Type", "PbrMaterial" },
        { "Color", ToJson(material->ObjectColor) },
        { "Metallic", material->Metallic },
        { "Roughness", material->Roughness },
        { "Albedo", CAST(u64, handle) }
    };

    auto full_path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    FileSystem::WriteEntireFile(full_path, j.dump(2));
}
