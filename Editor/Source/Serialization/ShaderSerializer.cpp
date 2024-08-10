#include "ShaderSerializer.h"

#include "Fussion/RHI/Resources/Resource.h"
#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/RHI/ShaderCompiler.h"
#include "Project/Project.h"

using namespace Fussion;

Ref<Asset> ShaderSerializer::Load(AssetMetadata metadata)
{
    // auto path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    // auto code = FileSystem::ReadEntireFile(path);
    // if (!code) {
    //     return nullptr;
    // }
    // auto result = RHI::ShaderCompiler::Compile(*code);
    return nullptr;
}
