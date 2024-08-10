#include "e5pch.h"
#include "ShaderCompiler.h"
#include "Device.h"
#include "OS/FileSystem.h"
#include "Vulkan/Common.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <sstream>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <cstring>

namespace Fussion::RHI {
    ElementType SpirvTypeToElementType(spirv_cross::SPIRType::BaseType base_type, u32 size)
    {
        switch (base_type) {
        case spirv_cross::SPIRType::Int:
            switch (size) {
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
                break;;
            }
            break;
        case spirv_cross::SPIRType::Float:
            switch (size) {
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
        default:
            break;
        }
        UNREACHABLE;
    }

    auto ReadPragma(std::string const& line) -> std::optional<ParsedPragma>
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

        ParsedPragma pragma;
        pragma.Key = key;
        pragma.Value = value;

        return pragma;
    };

    bool PreProcessShader(std::string const& source_code, std::string& out_vertex, std::string& out_fragment, std::vector<ParsedPragma>& parsed_pragmas)
    {
        std::istringstream is(source_code);

        auto current_shader = ShaderType::None;
        s32 current_line{ 1 };
        for (std::string line; std::getline(is, line); current_line++) {
            using namespace std::string_literals;
            static auto pragma_key = "#pragma"s;

            if (line.starts_with(pragma_key)) {
                if (auto pragma = ReadPragma(line.substr(pragma_key.size()))) {
                    if (pragma->Key == "type") {
                        if (pragma->Value == "vertex") {
                            current_shader = ShaderType::Vertex;
                            out_vertex += std::format("#line {}\n", current_line);
                        } else if (pragma->Value == "fragment") {
                            current_shader = ShaderType::Fragment;
                            out_fragment += std::format("#line {}\n", current_line);
                        }
                    } else {
                        parsed_pragmas.push_back(*pragma);
                    }
                }
                continue;
            }

            switch (current_shader) {
                using enum ShaderType;
            case None: {
                out_vertex += line + '\n';
                out_fragment += line + '\n';
            }
            break;
            case Vertex: {
                out_vertex += line + '\n';
            }
            break;
            case Fragment: {
                out_fragment += line + '\n';
            }
            break;
            default: ;
            }
        }
        return true;
    }

    class Includer final : public shaderc::CompileOptions::IncluderInterface {
        virtual shaderc_include_result* GetInclude(
            const char* requested_source, shaderc_include_type type,
            [[maybe_unused]] const char* requesting_source,
            [[maybe_unused]] size_t include_depth) override
        {
            auto result = new shaderc_include_result;
            switch (type) {
            case shaderc_include_type_relative: {
                auto full_path = std::filesystem::current_path() / "Assets" / std::string{ requested_source };
                ReadThingy(full_path, result);
            }
            break;
            case shaderc_include_type_standard: {
                auto base_path = std::filesystem::path("Assets/Shaders");
                auto full_path = base_path / std::string{ requested_source };
                ReadThingy(full_path, result);
            }
            break;
            }
            return result;
        }

        virtual void ReleaseInclude(shaderc_include_result* data) override
        {
            if (data->content_length != 0) {
                delete[] data->content;
            }
            if (data->source_name_length != 0) {
                delete[] data->source_name;
            }
        }

        static void ReadThingy(std::filesystem::path const& full_path, shaderc_include_result* result)
        {
            auto data = FileSystem::ReadEntireFile(full_path);
            if (!data.has_value()) {
                result->source_name = "";
                result->source_name_length = 0;

                result->content = "";
                result->content_length = 0;
            } else {
                auto str_path = full_path.string();
                auto cstr_path = new char[str_path.size()];
                std::memcpy(cstr_path, str_path.c_str(), str_path.size());

                result->source_name = cstr_path;
                result->source_name_length = str_path.size();

                auto cstr_content = new char[data->size()];
                std::memcpy(cstr_content, data->c_str(), data->size());
                result->content = cstr_content;
                result->content_length = data->size();
            }
        }
    };

    auto ShaderCompiler::Compile(std::string const& source_code, std::string const& file_name) -> Maybe<CompileResult>
    {
        std::vector<ShaderStage> ret;
        ShaderMetadata metadata;

        std::string vertex, fragment;
        PreProcessShader(source_code, vertex, fragment, metadata.ParsedPragmas);

        using namespace std::string_literals;

        if (auto pragma = std::ranges::find_if(metadata.ParsedPragmas, [](ParsedPragma const& pragma) {
            return pragma.Key == "samples";
        }); pragma != metadata.ParsedPragmas.end()) {
            auto samples = atoi(pragma->Value.data());
            metadata.Samples = CAST(u32, samples);
        }

        if (auto pragma = std::ranges::find_if(metadata.ParsedPragmas, [](ParsedPragma const& pragma) {
            return pragma.Key == "blending";
        }); pragma != metadata.ParsedPragmas.end()) {
            metadata.UseBlending = pragma->Value == "on";
        }

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // Needed to keep names.
        options.SetGenerateDebugInfo();
        options.SetPreserveBindings(true);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, VK_MAKE_VERSION(1, 3, 0));
        options.SetAutoBindUniforms(true);
        options.SetAutoMapLocations(true);
        options.AddMacroDefinition("Vertex", "main");
        options.AddMacroDefinition("Fragment", "main");

        auto includer = MakePtr<Includer>();
        options.SetIncluder(std::move(includer));

        auto result = compiler.CompileGlslToSpv(vertex.c_str(), vertex.size(), shaderc_vertex_shader, file_name.data(), options);
        VERIFY(result.GetCompilationStatus() != shaderc_compilation_status_internal_error);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOG_ERRORF("Shader compilation failed:\n{}", result.GetErrorMessage());
            return {};
        }

        auto vertex_stage = ShaderStage{
            .Type = ShaderType::Vertex,
            .Bytecode = { result.begin(), result.end() },
        };
        ret.emplace_back(vertex_stage);

        result = compiler.CompileGlslToSpv(fragment.c_str(), fragment.size(), shaderc_fragment_shader, file_name.data(), options);
        VERIFY(result.GetCompilationStatus() != shaderc_compilation_status_internal_error);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOG_ERRORF("Shader compilation failed:\n{}", result.GetErrorMessage());
            return {};
        }

        auto fragment_stage = ShaderStage{
            .Type = ShaderType::Fragment,
            .Bytecode = { result.begin(), result.end() },
        };
        ret.emplace_back(fragment_stage);

        auto device = Device::Instance();

        // ==============
        //  Vertex Stage
        // ==============
        {
            spirv_cross::Compiler reflection_compiler(vertex_stage.Bytecode);

            auto const resources = reflection_compiler.get_shader_resources();

            for (auto const& input : resources.stage_inputs) {
                auto location = reflection_compiler.get_decoration(input.id, spv::DecorationLocation);
                auto type = reflection_compiler.get_type(input.type_id);
                auto attribute_type = type.basetype;

                ElementType element = SpirvTypeToElementType(attribute_type, type.vecsize);
                LOG_DEBUGF("Vertex attribute at location {} with {} named {}", location, magic_enum::enum_name(element), input.name);
                metadata.VertexAttributes.push_back(VertexAttribute{
                    .Name = input.name,
                    .Type = element,
                    .Location = location,
                });
            }

            std::ranges::sort(metadata.VertexAttributes, [](VertexAttribute const& a, VertexAttribute const& b) {
                return a.Location < b.Location;
            });

            for (auto const& input : resources.uniform_buffers) {
                auto set = reflection_compiler.get_decoration(input.id, spv::DecorationDescriptorSet);
                auto binding = reflection_compiler.get_decoration(input.id, spv::DecorationBinding);

                if (metadata.Uniforms[set].contains(binding)) {
                    LOG_ERRORF("Shader delcared binding {}, of set {}, multiple times.", binding, set);
                    LOG_ERRORF("\tName: {}", input.name);
                }

                auto& usage = metadata.Uniforms[set][binding];
                usage.Label = input.name;
                usage.Type = ResourceType::UniformBuffer;
                usage.Count = 1;
                usage.Stages = ShaderType::Vertex;
            }

            for (auto const& push : resources.push_constant_buffers) {
                auto const& type = reflection_compiler.get_type(push.type_id);
                auto struct_size = reflection_compiler.get_declared_struct_size(type);
                LOG_DEBUGF("Push constant '{}' with size '{}'", push.name, struct_size);
                metadata.PushConstants.push_back(PushConstant{
                    .Stage = ShaderType::Vertex,
                    .Name = push.name,
                    .Size = struct_size,
                });
            }
        }

        // ==============
        // Fragment Stage
        // ==============
        {
            spirv_cross::Compiler reflection_compiler(fragment_stage.Bytecode);

            auto resources = reflection_compiler.get_shader_resources();
            for (auto const& input : resources.uniform_buffers) {
                auto set = reflection_compiler.get_decoration(input.id, spv::DecorationDescriptorSet);
                auto binding = reflection_compiler.get_decoration(input.id, spv::DecorationBinding);

                if (!metadata.Uniforms.contains(set)) {
                    if (!metadata.Uniforms[set].contains(binding)) {
                        auto& usage = metadata.Uniforms[set][binding];
                        usage.Label = input.name;
                        usage.Type = ResourceType::UniformBuffer;
                        usage.Count = 1;
                        usage.Stages = ShaderType::Fragment;
                    }
                } else {
                    if (!metadata.Uniforms[set].contains(binding)) {
                        auto& usage = metadata.Uniforms[set][binding];
                        usage.Label = input.name;
                        usage.Type = ResourceType::UniformBuffer;
                        usage.Count = 1;
                        usage.Stages = ShaderType::Fragment;
                    } else {
                        auto& usage = metadata.Uniforms[set][binding];
                        usage.Stages = usage.Stages | ShaderType::Fragment;
                    }
                }
            }

            for (auto const& image : resources.sampled_images) {
                auto set = reflection_compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
                auto binding = reflection_compiler.get_decoration(image.id, spv::DecorationBinding);

                if (!metadata.Uniforms.contains(set)) {
                    if (!metadata.Uniforms[set].contains(binding)) {
                        auto& usage = metadata.Uniforms[set][binding];
                        usage.Label = image.name;
                        usage.Type = ResourceType::CombinedImageSampler;
                        usage.Count = 1;
                        usage.Stages = ShaderType::Fragment;
                    }
                } else {
                    if (!metadata.Uniforms[set].contains(binding)) {
                        auto& usage = metadata.Uniforms[set][binding];
                        usage.Label = image.name;
                        usage.Type = ResourceType::CombinedImageSampler;
                        usage.Count = 1;
                        usage.Stages = ShaderType::Fragment;
                    } else {
                        auto& usage = metadata.Uniforms[set][binding];
                        usage.Stages = usage.Stages | ShaderType::Fragment;
                    }
                }
            }

            for (const auto& output : resources.stage_outputs) {
                auto location = reflection_compiler.get_decoration(output.id, spv::DecorationLocation);
                metadata.ColorOutputs.push_back(location);
            }

            for (auto const& push : resources.push_constant_buffers) {
                auto const& type = reflection_compiler.get_type(push.type_id);
                auto struct_size = reflection_compiler.get_declared_struct_size(type);
                auto pos = std::ranges::find_if(metadata.PushConstants, [&push](PushConstant const& pc) -> bool {
                    return pc.Name == push.name;
                });

                if (pos != metadata.PushConstants.end()) {
                    metadata.PushConstants.push_back(PushConstant{
                        .Stage = ShaderType::Fragment,
                        .Name = push.name,
                        .Size = struct_size,
                    });
                }
            }
        }

        return CompileResult{ ret, metadata };
    }
}
