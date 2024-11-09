#include "FussionPCH.h"
#include "Fussion/GPU/ShaderProcessor.h"
#include "Fussion/OS/FileSystem.h"
#include "tracy/Tracy.hpp"

#include <slang-com-ptr.h>
#include <slang.h>

namespace Fussion::GPU {
    auto ShaderProcessor::process_file(std::filesystem::path const& path) -> Maybe<std::string>
    {
        auto file = FileSystem::read_entire_file(path);
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
        pragma.key = key;
        pragma.value = value;

        return pragma;
    }

    bool PreProcessShader(
        std::string const& source_code,
        std::string& out_vertex,
        std::string& out_fragment,
        std::vector<ShaderProcessor::ParsedPragma>& parsed_pragmas
    )
    {
        std::istringstream is(source_code);

        auto currentShader = ShaderStage::None;
        s32 current_line { 1 };
        for (std::string line; std::getline(is, line); current_line++) {
            using namespace std::string_literals;
            static auto pragma_key = "#pragma"s;

            if (line.starts_with(pragma_key)) {
                if (auto pragma = ReadPragma(line.substr(pragma_key.size()))) {
                    if (pragma->key == "type") {
                        if (pragma->value == "vertex") {
                            currentShader = ShaderStage::Vertex;
                            out_vertex += std::format("#line {}\n", current_line);
                        } else if (pragma->value == "fragment") {
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

    ElementType SlangKindToElementType(slang::TypeReflection* type_reflection)
    {
        if (type_reflection->getKind() == slang::TypeReflection::Kind::Vector) {
            switch (type_reflection->getScalarType()) { // NOLINT(clang-diagnostic-switch-enum)
            case slang::TypeReflection::Int32:
                switch (type_reflection->getElementCount()) {
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
                switch (type_reflection->getElementCount()) {
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
                TODO();
            }
        } else {
            switch (type_reflection->getScalarType()) { // NOLINT(clang-diagnostic-switch-enum)
            case slang::TypeReflection::Int32:
                return ElementType::Int;
            case slang::TypeReflection::Float32:
                return ElementType::Float;
            default:
                TODO();
            }
        }
        UNREACHABLE();
    }

    ShaderProcessor::ShaderMetadata reflect_slang(
        slang::ProgramLayout* program_reflection,
        slang::IEntryPoint* vs_entry_point,
        slang::IEntryPoint* fs_entry_point
    )
    {
        ZoneScoped;
        using Slang::ComPtr;
        using namespace slang;
        ShaderProcessor::ShaderMetadata metadata {};

        {
            auto vsReflection = vs_entry_point->getLayout();
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
                        metadata.vertex_attributes.push_back(VertexAttribute {
                            .name = std::string(field->getName()),
                            .type = type,
                            .shader_location = j,
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

        if (fs_entry_point) {
            auto fragmentReflection = fs_entry_point->getLayout();
            auto entryPointReflection = fragmentReflection->getEntryPointByIndex(0);

            auto resultType = entryPointReflection->getResultVarLayout()->getTypeLayout();

            switch (resultType->getKind()) { // NOLINT(clang-diagnostic-switch-enum)
            case TypeReflection::Kind::Struct: {
                for (u32 i = 0; i < resultType->getFieldCount(); ++i) {
                    auto field = resultType->getFieldByIndex(i);
                    // auto fieldType = field->getType();

                    if (field->getSemanticName() && std::string(field->getSemanticName()) == "SV_TARGET") {
                        metadata.color_outputs.push_back(CAST(u32, field->getSemanticIndex()));
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
                metadata.color_outputs.push_back(0);
            } break;
            default:
                break;
            }
        }

        for (u32 i = 0; i < program_reflection->getParameterCount(); ++i) {
            VariableLayoutReflection* var = program_reflection->getParameterByIndex(i);

            auto category = var->getCategory();
            auto index = var->getBindingIndex();
            auto set = var->getBindingSpace(CAST(SlangParameterCategory, category));
            // LOG_INFOF("Category: {}", magic_enum::enum_name(category));
            //
            // LOG_INFOF("Slang var name: {}", var->getName());
            // LOG_INFOF("\tSet: {} | Binding: {}", set, index);

            ShaderProcessor::ResourceUsage resource_usage {
                .label = var->getName(),
                .stages = ShaderStage::Vertex,
                .binding = index,
            };

            switch (var->getType()->getKind()) {
            case TypeReflection::Kind::ConstantBuffer:
                resource_usage.stages = ShaderStage::Fragment | ShaderStage::Vertex;
                resource_usage.type = BindingType::Buffer {
                    .type = BufferBindingType::Uniform {},
                    .has_dynamic_offset = false,
                };
                break;
            case TypeReflection::Kind::Resource: {
                bool skip = false;
                TextureViewDimension view_dimension {};
                auto shape = var->getType()->getResourceShape();
                switch (shape) {
                case SLANG_TEXTURE_2D:
                    view_dimension = TextureViewDimension::D2;
                    break;
                case SLANG_TEXTURE_2D_ARRAY:
                    view_dimension = TextureViewDimension::D2_Array;
                    break;
                case SLANG_TEXTURE_3D:
                    view_dimension = TextureViewDimension::D3;
                    break;
                case SLANG_TEXTURE_CUBE:
                    view_dimension = TextureViewDimension::Cube;
                    break;
                case SLANG_TEXTURE_CUBE_ARRAY:
                    view_dimension = TextureViewDimension::CubeArray;
                    break;
                case SLANG_STRUCTURED_BUFFER: {
                    skip = true;
                    bool readOnly = var->getType()->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ;
                    resource_usage.stages = ShaderStage::Fragment | ShaderStage::Vertex;
                    resource_usage.type = BindingType::Buffer {
                        .type = BufferBindingType::Storage { .read_only = readOnly },
                        .has_dynamic_offset = false,
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
                        view_dimension = TextureViewDimension::D2_Array;
                    } else {
                        view_dimension = TextureViewDimension::D2;
                    }
                    resource_usage.stages = ShaderStage::Fragment;
                    resource_usage.type = BindingType::Texture {
                        .sample_type = TextureSampleType::Depth {},
                        .view_dimension = view_dimension, // Uninitialized access prevented by the skip check above.
                        .multi_sampled = false,
                    };
                    break;
                }

                resource_usage.stages = ShaderStage::Fragment;
                resource_usage.type = BindingType::Texture {
                    .sample_type = TextureSampleType::Float { .filterable = true },
                    .view_dimension = view_dimension, // Uninitialized access prevented by the skip check above.
                    .multi_sampled = false,
                };
            } break;
            case TypeReflection::Kind::SamplerState:
                resource_usage.stages = ShaderStage::Fragment;
                resource_usage.type = BindingType::Sampler {
                    .type = std::strcmp(var->getType()->getName(), "SamplerComparisonState") == 0 ? SamplerBindingType::Comparison : SamplerBindingType::Filtering,
                };
                break;
            case TypeReflection::Kind::ShaderStorageBuffer:
                TODO();
            default:
                TODO();
            }
            metadata.uniforms[cast<u32>(set)][cast<size_t>(index)] = resource_usage;
        }
        return metadata;
    }

    // FIXME: Crashes when freed
    struct SlangGlobalState {
        Slang::ComPtr<slang::IModule> common_module;
        Slang::ComPtr<slang::IGlobalSession> global_session {};
    } g_state;

    void ShaderProcessor::initialize()
    {
        ZoneScoped;
        using Slang::ComPtr;
        using namespace slang;

        {
            ZoneScopedN("Create Global Session");
            createGlobalSession(g_state.global_session.writeRef());
        }

        {
            SessionDesc sessionDesc = {};
            TargetDesc targetDesc = {};
            targetDesc.format = SLANG_SPIRV;
            targetDesc.profile = g_state.global_session->findProfile("spirv_1_6");
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
            SLANG_CHECK(g_state.global_session->createSession(sessionDesc, session.writeRef()));

            auto DiagnoseIfNeeded = [](IBlob* blob) {
                if (blob != nullptr) {
                    LOG_WARNF("{}", std::string_view(CAST(char const*, blob->getBufferPointer()), blob->getBufferSize()));
                }
            };

            ZoneScopedN("Common Module");
            ComPtr<IBlob> diagnosticBlob;
            auto commonSrc = FileSystem::read_entire_file("Assets/Shaders/Slang/Common.slang").unwrap();
            g_state.common_module = session->loadModuleFromSourceString("common", "Assets/Shaders/Slang/Common.slang", commonSrc.data(), diagnosticBlob.writeRef());
            DiagnoseIfNeeded(diagnosticBlob);
        }
    }

    void ShaderProcessor::shutdown()
    {
        g_state.common_module->Release();
        g_state.global_session->Release();
    }

    auto ShaderProcessor::compile_slang(std::filesystem::path const& path) -> Maybe<CompiledShader>
    {
        ZoneScoped;
        using Slang::ComPtr;
        using namespace slang;

        SessionDesc sessionDesc = {};
        TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = g_state.global_session->findProfile("spirv_1_6");
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
        auto shaders_folder = (curr / "Assets" / "Shaders" / "Slang").string();
        char const* paths[] = { shaders_folder.c_str() };
        sessionDesc.searchPaths = paths;
        sessionDesc.searchPathCount = sizeof(paths) / sizeof(char const*);

        ComPtr<ISession> session;
        SLANG_CHECK(g_state.global_session->createSession(sessionDesc, session.writeRef()));

        auto diagnose_if_needed = [](IBlob* blob) {
            if (blob != nullptr) {
                LOG_WARNF("{}", std::string_view(CAST(char const*, blob->getBufferPointer()), blob->getBufferSize()));
            }
        };

        auto source = FileSystem::read_entire_file(path).unwrap();

        ComPtr<IModule> slang_module = nullptr;
        {
            ZoneScopedN("Slang Module");
            ComPtr<IBlob> diagnosticBlob;
            slang_module = session->loadModuleFromSourceString("my_module", path.string().c_str(), source.c_str(), diagnosticBlob.writeRef());
            diagnose_if_needed(diagnosticBlob);
            if (!slang_module)
                return None();
        }

        ComPtr<IEntryPoint> vs_entry_point;
        ComPtr<IEntryPoint> fs_entry_point;
        slang_module->findEntryPointByName("VS_Main", vs_entry_point.writeRef());
        slang_module->findEntryPointByName("FS_Main", fs_entry_point.writeRef());

        std::vector<IComponentType*> componentTypes;
        componentTypes.push_back(g_state.common_module);
        componentTypes.push_back(slang_module);
        componentTypes.push_back(vs_entry_point);

        if (fs_entry_point) {
            componentTypes.push_back(fs_entry_point);
        }

        ComPtr<IComponentType> composed_program;
        {
            ZoneScopedN("Composed Program");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = session->createCompositeComponentType(
                componentTypes.data(),
                CAST(SlangInt, componentTypes.size()),
                composed_program.writeRef(),
                diagnosticsBlob.writeRef()
            );
            diagnose_if_needed(diagnosticsBlob);
            SLANG_CHECK(result);
        }

        ComPtr<IComponentType> linked_program;
        {
            ZoneScopedN("Linked Program");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = composed_program->link(linked_program.writeRef(), diagnosticsBlob.writeRef());
            diagnose_if_needed(diagnosticsBlob);
            SLANG_CHECK(result);
        }

        CompiledShader shader {};
        ComPtr<IBlob> spirv_blob;
        {
            ZoneScopedN("VS Entry Point Code");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = linked_program->getEntryPointCode(
                0, 0, spirv_blob.writeRef(), diagnosticsBlob.writeRef()
            );
            diagnose_if_needed(diagnosticsBlob);
            SLANG_CHECK(result);

            usz size = spirv_blob->getBufferSize() / 4;
            shader.vertex_stage.resize(size);
            auto const* ptr = CAST(u32 const*, spirv_blob->getBufferPointer());
            std::copy_n(ptr, size, shader.vertex_stage.data());
        }

        ComPtr<IBlob> fs_spirv_blob;
        if (fs_entry_point) {
            ZoneScopedN("FS Entry Point Code");
            ComPtr<IBlob> diagnosticsBlob;
            SlangResult result = linked_program->getEntryPointCode(
                1, 0, fs_spirv_blob.writeRef(), diagnosticsBlob.writeRef()
            );
            diagnose_if_needed(diagnosticsBlob);
            SLANG_CHECK(result);

            usz size = fs_spirv_blob->getBufferSize() / 4;
            shader.fragment_stage.resize(size);
            auto const* ptr = CAST(u32 const*, fs_spirv_blob->getBufferPointer());
            std::copy_n(ptr, size, shader.fragment_stage.data());
        }

        ProgramLayout* reflection = slang_module->getLayout();

        shader.metadata = reflect_slang(reflection, vs_entry_point, fs_entry_point);

        return shader;
    }
}
