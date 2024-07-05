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

        material->Color = j["Color"].get<Color>();
    } catch (std::exception const& e) {
        LOG_ERRORF("Error while deserializing PbrMaterial: {}", e.what());
    }
    return material;
}

void PbrMaterialSerializer::Save(AssetMetadata metadata, Ref<Asset> const& asset)
{
    auto material = asset->As<PbrMaterial>();
    ordered_json j = {
        {"$Type", "PbrMaterial"},
        {"Color", ToJson(material->Color)},
    };

    auto full_path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    FileSystem::WriteEntireFile(full_path, j.dump(2));
}
