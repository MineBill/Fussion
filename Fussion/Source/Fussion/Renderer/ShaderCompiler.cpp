#include "e5pch.h"
#include "ShaderCompiler.h"
#include "Device.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <sstream>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

namespace Fussion
{
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
            }
            break;
        default:
            PANIC("Unhandled case for SPIRV type: {}", magic_enum::enum_name(base_type));
            break;
        }
    }

    bool SplitShader(std::string const& source_code, std::string& out_vertex, std::string& out_fragment)
    {
        std::istringstream is(source_code);

        auto current_shader = ShaderType::None;
        s32 vertex_line{}, fragment_line{};
        s32 current_line{1};
        for (std::string line; std::getline(is, line); current_line++) {
            if (line.starts_with("#pragma")) {
                auto pos = line.find(':');
                auto type = line.substr(pos + 1);

                size_t i;
                while ((i = type.find_first_of(' ')) != std::string::npos)
                    type.erase(i, 1);
                if (type == "vertex") {
                    current_shader = ShaderType::Vertex;
                    out_vertex += std::format("#line {}\n", current_line);
                } else if (type == "fragment") {
                    current_shader = ShaderType::Fragment;
                    out_fragment += std::format("#line {}\n", current_line);
                }
                continue;
            }

            switch(current_shader) {
            using enum ShaderType;
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
            default: ;
            }
        }
        return true;
    }

    auto ShaderCompiler::Compile(std::string const& source_code) -> std::tuple<std::vector<ShaderStage>, ShaderMetadata>
    {
        std::vector<ShaderStage> ret;
        ShaderMetadata metadata;

        std::string vertex, fragment;
        SplitShader(source_code, vertex, fragment);

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // Needed to keep names.
        options.SetGenerateDebugInfo();
        options.SetPreserveBindings(true);
        options.SetVulkanRulesRelaxed(true);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        auto result = compiler.CompileGlslToSpv(vertex.c_str(), vertex.size(), shaderc_vertex_shader, "file.shader", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOG_ERRORF("Shader compilation failed: {}", result.GetErrorMessage());
            return {};
        }

        auto vertex_stage = ShaderStage{
            .Type = ShaderType::Vertex,
            .Bytecode = {result.begin(), result.end()},
        };
        ret.emplace_back(vertex_stage);

        result = compiler.CompileGlslToSpv(fragment.c_str(), fragment.size(), shaderc_fragment_shader, "file.shader", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOG_ERRORF("Shader compilation failed: {}", result.GetErrorMessage());
            return {};
        }

        auto fragment_stage = ShaderStage{
            .Type = ShaderType::Fragment,
            .Bytecode = {result.begin(), result.end()},
        };
        ret.emplace_back(fragment_stage);

        auto device = Device::Instance();

        {
            spirv_cross::CompilerGLSL reflection_compiler(vertex_stage.Bytecode);

            auto resources = reflection_compiler.get_shader_resources();
            for (const auto& input : resources.stage_inputs) {
                auto location = reflection_compiler.get_decoration(input.id, spv::DecorationLocation);
                auto type = reflection_compiler.get_type(input.type_id);
                auto attribute_type = type.basetype;

                ElementType element = SpirvTypeToElementType(attribute_type, type.vecsize);
                LOG_DEBUGF("Vertex attribute at location {} with {}", location, magic_enum::enum_name(element));
                metadata.VertexAttributes.push_back(VertexAttribute {
                    .Name = input.name,
                    .Type = element,
                });
            }

            for (const auto& input : resources.uniform_buffers) {
                auto set = reflection_compiler.get_decoration(input.id, spv::DecorationDescriptorSet);
                auto binding = reflection_compiler.get_decoration(input.id, spv::DecorationBinding);

                if (!metadata.Uniforms.contains(set)) {
                    if (!metadata.Uniforms[set].contains(binding)) {
                        auto& usage = metadata.Uniforms[set][binding];
                        usage.Label = input.name;
                        usage.Type = ResourceType::UniformBuffer;
                        usage.Count = 1;
                        usage.Stages = ShaderType::Vertex;
                    }
                }
            }
        }

        {
            spirv_cross::CompilerGLSL reflection_compiler(fragment_stage.Bytecode);

            auto resources = reflection_compiler.get_shader_resources();
            for (const auto& input : resources.uniform_buffers) {
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

            for (const auto& output: resources.stage_outputs) {
                auto location = reflection_compiler.get_decoration(output.id, spv::DecorationLocation);
                metadata.ColorOutputs.push_back(location);
            }
        }

        // std::vector<ResourceUsage> flat_resource_usages;
        // LOG_DEBUG("Shader uses these uniform buffers:");
        // for (const auto& [set, bindings] : metadata.Uniforms) {
        //     LOG_DEBUGF("\tFor set({}):", set);
        //     for (const auto& [binding, usage] : bindings) {
        //         LOG_DEBUGF("\t\tBinding {}, {}, {}, {}", binding, usage.Label, magic_enum::enum_name(usage.Type), usage.Stages.value);
        //         // flat_resource_usages.push_back(usage);
        //     }
        // }

        // auto layout = device->CreateResourceLayout(flat_resource_usages);
        //
        // PipelineLayoutSpecification layout_spec;
        //
        // layout_spec.ResourceUsages.emplace_back();
        //
        // PipelineSpecification spec{
        //     .Label = "",
        //     .AttributeLayout = VertexAttributeLayout::Create({}),
        // };

        return {ret, metadata};
    }
}