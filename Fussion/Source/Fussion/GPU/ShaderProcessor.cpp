#include "FussionPCH.h"
#include "Fussion/GPU/ShaderProcessor.h"
#include "Fussion/OS/FileSystem.h"
#include "tracy/Tracy.hpp"

#include <slang-com-ptr.h>
#include <slang.h>

namespace Fussion::GPU {
    auto ShaderProcessor::ProcessFile(std::filesystem::path const& path) -> Maybe<std::string>
    {
        auto file = FileSystem::ReadEntireFile(path);
        if (!file) {
            return None();
        }

        std::stringstream ss;

        std::istringstream iss(*file);
        for (std::string line; std::getline(iss, line);) {
            if (!line.starts_with("#")) {
                ss << line << '\n';
            }
        }
        return ss.str();
    }

    auto ReadPragma(std::string const& line) -> std::optional<ShaderProcessor::ParsedPragma>
    {
        auto pos = line.find(':');
        if (pos == std::string::npos) {
            return std::nullopt;
        }

        auto key = line.substr(0, pos);
        auto value = line.substr(pos + 1);

        // Clear whitespace
        size_t i;
        while ((i = value.find_first_of(' ')) != std::string::npos)
            value.erase(i, 1);
        while ((i = key.find_first_of(' ')) != std::string::npos)
            key.erase(i, 1);

        ShaderProcessor::ParsedPragma pragma;
        pragma.Key = key;
        pragma.Value = value;

        return pragma;
    }

    bool PreProcessShader(
        std::string const& source_code,
        std::string& out_vertex,
        std::string& out_fragment,
        std::vector<ShaderProcessor::ParsedPragma>& parsed_pragmas)
    {
        std::istringstream is(source_code);

        auto currentShader = ShaderStage::None;
        s32 current_line { 1 };
        for (std::string line; std::getline(is, line); current_line++) {
            using namespace std::string_literals;
            static auto pragma_key = "#pragma"s;

            if (line.starts_with(pragma_key)) {
                if (auto pragma = ReadPragma(line.substr(pragma_key.size()))) {
                    if (pragma->Key == "type") {
                        if (pragma->Value == "vertex") {
                            currentShader = ShaderStage::Vertex;
                            out_vertex += std::format("#line {}\n", current_line);
                        } else if (pragma->Value == "fragment") {
                            currentShader = ShaderStage::Fragment;
                            out_fragment += std::format("#line {}\n", current_line);
                        }
                    } else {
                        parsed_pragmas.push_back(*pragma);
                    }
                }
                continue;
            }

            switch (currentShader) {
                using enum ShaderStage;
            case None: {
                out_vertex += line + '\n';
                out_fragment += line + '\n';
            } break;
            case Vertex: {
                out_vertex += line + '\n';
            } break;
            case Fragment: {
                out_fragment += line + '\n';
            } break;
            default:;
            }
        }
        return true;
    }

#define SLANG_CHECK(result) VERIFY(SLANG_SUCCEEDED(result))

    ElementType SlangKindToElementType(slang::TypeReflection* typeReflection)
    {
        if (typeReflection->getKind() == slang::TypeReflection::Kind::Vector) {
            switch (typeReflection->getScalarType()) { // NOLINT(clang-diagnostic-switch-enum)
            case slang::TypeReflection::Int32:
                switch (typeReflection->getElementCount()) {
                case 0:
                case 1:
                    return ElementType::Int;
                case 2:
                    return ElementType::Int2;
                case 3:
                    return ElementType::Int3;
                case 4:
                    return ElementType::Int4;
                default:
                    break;
                }
                break;
            case slang::TypeReflection::Float32:
                switch (typeReflection->getElementCount()) {
                case 0:
                case 1:
                    return ElementType::Float;
                case 2:
                    return ElementType::Float2;
                case 3:
                    return ElementType::Float3;
                case 4:
                    return ElementType::Float4;
                default:
                    break;
                }
                break;
                // case slang::TypeReflection::UInt32:
                //     switch (typeReflection->getElementCount()) {
                // case 0:
                // case 1:
                //     return ElementType::Int;
                // case 2:
                //     return ElementType::Int2;
                // case 3:
                //     return ElementType::Int3;
                // default:
                //     break;
                //     }
                //     break;
            default:
                UNIMPLEMENTED;
            }
        } else {
            switch (typeReflection->getScalarType()) { // NOLINT(clang-diagnostic-switch-enum)
            case slang::TypeReflection::Int32:
                return ElementType::Int;
            case slang::TypeReflection::Float32:
                return ElementType::Float;
            default:
                UNIMPLEMENTED;
            }
        }
        UNREACHABLE;
    }

    ShaderProcessor::ShaderMetadata ReflectSlang(
        slang::ProgramLayout* programReflection,
        slang::IEntryPoint* vsEntryPoint,
        slang::IEntryPoint* fsEntryPoint)
    {
        ZoneScoped;
        using Slang::ComPtr;
        using namespace slang;
        ShaderProcessor::ShaderMetadata metadata {};

        {
            auto vsReflection = vsEntryPoint->getLayout();
            auto entryPointReflection = vsReflection->getEntryPointByIndex(0);

            // Iterate over the VS_Main inputs. Usually this will only be one, usually of type VSOuput.
            for (u32 i = 0; i < entryPointReflection->getParameterCount(); ++i) {
                VariableLayoutReflection* parameter = entryPointReflection->getParameterByIndex(i);
                if (parameter->getSemanticName() != nullptr) {
                    continue;
                }
                TypeLayoutReflection* parameterType = parameter->getTypeLayout();

                // Iterate over the input struct, e.g. VSOutput
                switch (parameterType->getKind()) { // NOLINT(clang-diagnostic-switch-enum)
                case TypeReflection::Kind::Struct: {
                    for (u32 j = 0; j < parameterType->getFieldCount(); ++j) {
                        auto field = parameterType->getFieldByIndex(j);
                        auto fieldType = field->getType();

                        // We only handle vectors and scalars for now.
                        if (fieldType->getKind() != TypeReflection::Kind::Vector && fieldType->getKind() != TypeReflection::Kind::Scalar) {
                            LOG_ERRORF("Unsupported field type kind {}", magic_enum::enum_name(fieldType->getKind()));
                        }

                        ElementType type = SlangKindToElementType(fieldType);
                        metadata.VertexAttributes.push_back(VertexAttribute {
                            .Name = std::string(field->getName()),
                            .Type = type,
                            .ShaderLocation = j,
                        });
                    }
                } break;
                default: {
                    LOG_WARNF("Unknown type {} used vertex shader entry point.", magic_enum::enum_name(parameterType->getKind()));
                } break;
                }
                // for (u32 i = 0; i < parameter->getTypeLayout()->getFieldCount(); ++i) {
                //     auto field = parameter->getTypeLayout()->getFieldByIndex(i);
                // LOG_INFO("============================");
                // LOG_INFOF("{}", field->getName());
                // auto fieldType = field->getType();
                // LOG_INFOF("{}", fieldType->getRowCount());
                // LOG_INFOF("{}", ENUMNAME(fieldType->getResourceAccess()));
                // LOG_INFOF("{}", ENUMNAME(fieldType->getKind()));
                // LOG_INFOF("{}", ENUMNAME(fieldType->getResourceShape()));
                // LOG_INFOF("{}", fieldType->getTotalArrayElementCount());
                // if (field->getSemanticName()) {
                //     LOG_INFOF("{}{}", field->getSemanticName(), field->getSemanticIndex());
                // }
                //
                // // for stuff like vector
                // if (fieldType->getKind() == TypeReflection::Kind::Vector) {
                //     LOG_INFOF("{}", ENUMNAME(fieldType->getScalarType()));
                //     LOG_INFOF("{}", fieldType->getElementCount());
                //     LOG_INFOF("{}", ENUMNAME(fieldType->getElementType()->getKind()));
                // }
                // }
            }
        }

        if (fsEntryPoint) {
            auto fragmentReflection = fsEntryPoint->getLayout();
            auto entryPointReflection = fragmentReflection->getEntryPointByIndex(0);

            auto resultType = entryPointReflection->getResultVarLayout()->getTypeLayout();

            switch (resultType->getKind()) { // NOLINT(clang-diagnostic-switch-enum)
            case TypeReflection::Kind::Struct: {
                for (u32 i = 0; i < resultType->getFieldCount(); ++i) {
                    auto field = resultType->getFieldByIndex(i);
                    // auto fieldType = field->getType();

                    if (field->getSemanticName() && std::string(field->getSemanticName()) == "SV_TARGET") {
                        metadata.ColorOutputs.push_back(CAST(u32, field->getSemanticIndex()));
                    }
                    // LOG_INFO("============================");
                    // LOG_INFOF("{}", field->getName());
                    // LOG_INFOF("{}", fieldType->getRowCount());
                    // LOG_INFOF("{}", ENUMNAME(fieldType->getResourceAccess()));
                    // LOG_INFOF("{}", ENUMNAME(fieldType->getKind()));
                    // LOG_INFOF("{}", ENUMNAME(fieldType->getResourceShape()));
                    // LOG_INFOF("{}", fieldType->getTotalArrayElementCount());
                    // LOG_INFOF("{}{}", field->getSemanticName(), field->getSemanticIndex());
                    //
                    // // for stuff like vector
                    // if (fieldType->getKind() == TypeReflection::Kind::Vector) {
                    //     LOG_INFOF("{}", ENUMNAME(fieldType->getScalarType()));
                    //     LOG_INFOF("{}", fieldType->getElementCount());
                    //     LOG_INFOF("{}", ENUMNAME(fieldType->getElementType()->getKind()));
                    // }
                }
            } break;
            case TypeReflection::Kind::Scalar:
            case TypeReflection::Kind::Vector: {
                metadata.ColorOutputs.push_back(0);
            } break;
            default:
                break;
            }
        }

        for (u32 i = 0; i < programReflection->getParameterCount(); ++i) {
            VariableLayoutReflection* var = programReflection->getParameterByIndex(i);

            auto category = var->getCategory();
            auto index = var->getBindingIndex();
            auto set = var->getBindingSpace(CAST(SlangParameterCategory, category));
            LOG_INFOF("Category: {}", magic_enum::enum_name(category));

            LOG_INFOF("Slang var name: {}", var->getName());
            LOG_INFOF("\tSet: {} | Binding: {}", set, index);

            ShaderProcessor::ResourceUsage resourceUsage {
                .Label = var->getName(),
                .Stages = ShaderStage::Vertex,
                .Binding = index,
            };

            switch (var->getType()->getKind()) {
            case TypeReflection::Kind::ConstantBuffer:
                resourceUsage.Stages = ShaderStage::Fragment | ShaderStage::Vertex;
                resourceUsage.Type = BindingType::Buffer {
                    .Type = BufferBindingType::Uniform {},
                    .HasDynamicOffset = false,
                };
                break;
            case TypeReflection::Kind::Resource: {
                bool skip = false;
                TextureViewDimension viewDimension {};
                auto shape = var->getType()->getResourceShape();
                switch (shape) {
                case SLANG_TEXTURE_2D:
                    viewDimension = TextureViewDimension::D2;
                    break;
                case SLANG_TEXTURE_2D_ARRAY:
                    viewDimension = TextureViewDimension::D2_Array;
                    break;
                case SLANG_TEXTURE_3D:
                    viewDimension = TextureViewDimension::D3;
                    break;
                case SLANG_TEXTURE_CUBE:
                    viewDimension = TextureViewDimension::Cube;
                    break;
                case SLANG_TEXTURE_CUBE_ARRAY:
                    viewDimension = TextureViewDimension::CubeArray;
                    break;
                case SLANG_STRUCTURED_BUFFER: {
                    skip = true;
                    bool readOnly = var->getType()->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ;
                    resourceUsage.Stages = ShaderStage::Fragment | ShaderStage::Vertex;
                    resourceUsage.Type = BindingType::Buffer {
                        .Type = BufferBindingType::Storage { .ReadOnly = readOnly },
                        .HasDynamicOffset = false,
                    };
                } break;
                default:
                    break;
                    // PANIC("Unimplemented: {}", magic_enum::enum_name(var->getType()->getResourceShape()));
                }
                if (skip) {
                    break;
                }
                if (shape & SLANG_TEXTURE_SHADOW_FLAG) {
                    if (shape & SLANG_TEXTURE_ARRAY_FLAG) {
                        viewDimension = TextureViewDimension::D2_Array;
                    } else {
                        viewDimension = TextureViewDimension::D2;
                    }
                    resourceUsage.Stages = ShaderStage::Fragment;
                    resourceUsage.Type = BindingType::Texture {
                        .SampleType = TextureSampleType::Depth {},
                        .ViewDimension = viewDimension, // Uninitialized access prevented by the skip check above.
                        .MultiSampled = false,
                    };
                    break;
                }

                resourceUsage.Stages = ShaderStage::Fragment;
                resourceUsage.Type = BindingType::Texture {
                    .SampleType = TextureSampleType::Float { .Filterable = true },
                    .ViewDimension = viewDimension, // Uninitialized access prevented by the skip check above.
                    .MultiSampled = false,
                };
            } break;
            case TypeReflection::Kind::SamplerState:
                resourceUsage.Stages = ShaderStage::Fragment;
                resourceUsage.Type = BindingType::Sampler {
                    .Type = SamplerBindingType::Filtering,
                };
                break;
            case TypeReflection::Kind::ShaderStorageBuffer:
                UNIMPLEMENTED;
            default:
                UNIMPLEMENTED;
            }
            metadata.Uniforms[set][CAST(usz, index)] = resourceUsage;
        }
        return metadata;
    }

    Slang::ComPtr<slang::IModule> g_CommonModule;
    Slang::ComPtr<slang::IGlobalSession> g_GlobalSession {};

    auto ShaderProcessor::CompileSlang(std::filesystem::path const& path) -> Maybe<CompiledShader>
    {
        ZoneScoped;
        using Slang::ComPtr;
        using namespace slang;

        if (!g_GlobalSession) {
            ZoneScopedN("Create Global Session");
            createGlobalSession(g_GlobalSession.writeRef());
        }

        SessionDesc sessionDesc = {};
        TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = g_GlobalSession->findProfile("spirv_1_5");
        targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

        CompilerOptionEntry entries[] = {
            { CompilerOptionName::MinimumSlangOptimization, { .intValue0 = 1 } },
            { CompilerOptionName::ReportDownstreamTime, { .intValue0 = 1 } },
        };
        targetDesc.compilerOptionEntries = entries;
        targetDesc.compilerOptionEntryCount = sizeof(entries) / sizeof(CompilerOptionEntry);

        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        sessionDesc.compilerOptionEntries = entries;
        sessionDesc.compilerOptionEntryCount = sizeof(entries) / sizeof(CompilerOptionEntry);

        auto curr = std::filesystem::current_path();
        auto shadersFolder = (curr / "Assets" / "Shaders" / "Slang").string();
        char const* paths[] = { shadersFolder.c_str() };
        sessionDesc.searchPaths = paths;
        sessionDesc.searchPathCount = sizeof(paths) / sizeof(char const*);

        ComPtr<ISession> session;
        SLANG_CHECK(g_GlobalSession->createSession(sessionDesc, session.writeRef()));

        auto DiagnoseIfNeeded = [](IBlob* blob) {
            if (blob != nullptr) {
                LOG_WARNF("{}", std::string_view(CAST(char const*, blob->getBufferPointer()), blob->getBufferSize()));
            }
        };

        if (!g_CommonModule) {
            ZoneScopedN("Common Module");
            ComPtr<IBlob> diagnosticBlob;
            auto commonSrc = FileSystem::ReadEntireFile("Assets/Shaders/Slang/Common.slang").Unwrap();
            g_CommonModule = session->loadModuleFromSourceString("common", "Assets/Shaders/Slang/Common.slang", commonSrc.data(), diagnosticBlob.writeRef());
            DiagnoseIfNeeded(diagnosticBlob);
            if (!g_CommonModule)
                return None();
        }

        auto source = FileSystem::ReadEntireFile(path).Unwrap();

        ComPtr<IModule> slangModule = nullptr;
        {
            ZoneScopedN("Slang Module");
            ComPtr<IBlob> diagnosticBlob;
            slangModule = session->loadModuleFromSourceString("my_module", path.string().c_str(), source.c_str(), diagnosticBlob.writeRef());
            DiagnoseIfNeeded(diagnosticBlob);
            if (!slangModule)
                return None();
        }

        ComPtr<IEntryPoint> vsEntryPoint;
        ComPtr<IEntryPoint> fsEntryPoint;
        slangModule->findEntryPointByName("VS_Main", vsEntryPoint.writeRef());
        slangModule->findEntryPointByName("FS_Main", fsEntryPoint.writeRef());

        std::vector<IComponentType*> componentTypes;
        componentTypes.push_back(g_CommonModule);
        componentTypes.push_back(slangModule);
        componentTypes.push_back(vsEntryPoint);

        if (fsEntryPoint) {
            componentTypes.push_back(fsEntryPoint);
        }

        ComPtr<IComponentType> composedProgram;
        {
            ZoneScopedN("Composed Program");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = session->createCompositeComponentType(
                componentTypes.data(),
                CAST(SlangInt, componentTypes.size()),
                composedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            DiagnoseIfNeeded(diagnosticsBlob);
            SLANG_CHECK(result);
        }

        ComPtr<IComponentType> linkedProgram;
        {
            ZoneScopedN("Linked Program");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
            DiagnoseIfNeeded(diagnosticsBlob);
            SLANG_CHECK(result);
        }

        CompiledShader shader {};
        ComPtr<IBlob> spirvBlob;
        {
            ZoneScopedN("VS Entry Point Code");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = linkedProgram->getEntryPointCode(
                0, 0, spirvBlob.writeRef(), diagnosticsBlob.writeRef());
            DiagnoseIfNeeded(diagnosticsBlob);
            SLANG_CHECK(result);

            usz size = spirvBlob->getBufferSize() / 4;
            shader.VertexStage.resize(size);
            auto const* ptr = CAST(u32 const*, spirvBlob->getBufferPointer());
            std::copy_n(ptr, size, shader.VertexStage.data());
        }

        ComPtr<IBlob> fsSpirvBlob;
        if (fsEntryPoint) {
            ZoneScopedN("FS Entry Point Code");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = linkedProgram->getEntryPointCode(
                1, 0, fsSpirvBlob.writeRef(), diagnosticsBlob.writeRef());
            DiagnoseIfNeeded(diagnosticsBlob);
            SLANG_CHECK(result);

            usz size = fsSpirvBlob->getBufferSize() / 4;
            shader.FragmentStage.resize(size);
            auto const* ptr = CAST(u32 const*, fsSpirvBlob->getBufferPointer());
            std::copy_n(ptr, size, shader.FragmentStage.data());
        }

        ProgramLayout* reflection = slangModule->getLayout();

        shader.Metadata = ReflectSlang(reflection, vsEntryPoint, fsEntryPoint);

        return shader;
    }
}
