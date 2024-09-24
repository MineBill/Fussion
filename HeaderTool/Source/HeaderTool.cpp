#include "Query.h"
#include "../../Vendor/magic_enum/include/magic_enum/magic_enum.hpp"

#include <tree_sitter/api.h>
#include <tree-sitter-cpp.h>

#include <tracy/Tracy.hpp>

#include <optional>
#include <string>
#include <filesystem>
#include <fstream>
#include <format>
#include <print>
#include <tree.h>
#include <algorithm>
#include <ranges>
#include <cstring>
#include <set>

constexpr auto CLASS_QUERY = R"fennel(
(
 [(struct_specifier
    (attribute_declaration
      (attribute
        prefix: (identifier)? @attr-prefix
        name: (identifier) @attr-name
        (argument_list
          (_))? @attr-arguments))
    name: (type_identifier) @class-name
    (base_class_clause
      [
       (type_identifier) @base-class
       (qualified_identifier
         name: (type_identifier) @base-class)
       ])?
    )
  (class_specifier
    (attribute_declaration
      (attribute
        prefix: (identifier)? @attr-prefix
        name: (identifier) @attr-name
        (argument_list
          (_))? @attr-arguments))
    name: (type_identifier) @class-name
    (base_class_clause
      [
       (type_identifier) @base-class
       (qualified_identifier
         name: (type_identifier) @base-class)
       ])?
    )]
 ) @class
)fennel";

constexpr auto FIELD_QUERY = R"fennel(
(
 (comment)? @field-comment
 .
 (field_declaration
   (attribute_declaration
     (attribute
       prefix: (identifier)? @attr-prefix
       name: (identifier) @attr-name
       (argument_list
         (_))? @attr-arguments))
   declarator: (field_identifier) @field-name) @field
))fennel";

constexpr auto METHOD_QUERY = R"fennel(
(
 (comment)? @field-comment
 .
 [
  (field_declaration
    (attribute_declaration
      (attribute
        prefix: (identifier)? @attr-prefix
        name: (identifier) @attr-name
        (argument_list
          (_))? @attr-arguments))

    declarator: (function_declarator
                  declarator: (field_identifier) @method-name
                  parameters: (parameter_list
                                (
                                 (parameter_declaration
                                   declarator: _ @param-name)
                                 ","?
                                 )*
                                ) @params
                  )
    ) @method

  (function_definition
    (attribute_declaration
      (attribute
        prefix: (identifier)? @attr-prefix
        name: (identifier) @attr-name
        (argument_list
          (_))? @attr-arguments))

    declarator: (function_declarator
                  declarator: (field_identifier) @method-name
                  parameters: (parameter_list
                                (
                                 (parameter_declaration
                                   declarator: _ @param-name)
                                 ","?
                                 )*
                                ) @params
                  )
    ) @method
  ]
 )
)fennel";

constexpr auto ENUM_QUERY = R"fennel(
(enum_specifier
  (attribute_declaration
    (attribute
        prefix: (identifier)? @attr-prefix
        name: (identifier) @attr-name
        (argument_list
          (_))? @arguments))
  name: (type_identifier) @name) @enum
)fennel";

std::optional<std::string> read_entire_file(std::filesystem::path const& path)
{
    if (!exists(path)) {
        return std::nullopt;
    }

    std::ifstream file(path);
    return std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
}

struct Attribute {
    std::string prefix{};
    std::string name{};
    std::string args{};
};

struct ReflectedType {
    std::unordered_map<std::string, Attribute> attributes{};

    void update_attribute(std::string const& name, std::string const& prefix, std::string const& args)
    {
        attributes[name].name = name;
        attributes[name].prefix = prefix;
        if (!args.empty())
            attributes[name].args = args.substr(1, args.size() - 2);

    }

    bool has_attribute(std::string const& name) const
    {
        return attributes.contains(name);
    }
};

struct ReflectedClass;

struct ClassMember : ReflectedType {
    std::string name{};

    TSNode node{};
};

struct ClassMethod : ReflectedType {
    std::string name{};

    TSNode node{};
};

struct ReflectedClass : ReflectedType {
    std::string name{};
    std::string namespace_{};

    std::vector<ClassMember> members{};
    std::vector<ClassMethod> methods{};

    TSNode class_node{};

    /// Component classes are a bit special in code generation
    /// so we track them here.
    bool is_component{};

    std::string qualified_name() const
    {
        return namespace_ + "::" + name;
    }

};

struct Enum {
    std::string name{};
    std::string qualified_name{};
    std::unordered_map<std::string, Attribute> attributes{};

    TSNode node{};
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::println("[HeaderTool] Output file not provided");
        return -1;
    }

    std::println("[HeaderTool] Generating reflection information...");
    auto output_path = std::filesystem::path(argv[1]) / "ReflectionRegistry_Generated.cpp";
    ZoneScoped;
    auto parser = ts_parser_new();

    using namespace std::string_view_literals;
    std::set files_to_ignore{
        "stb_image.h"sv,
    };

    ts_parser_set_language(parser, tree_sitter_cpp());

    std::unordered_map<std::string, ReflectedClass> attribute_classes{};
    std::vector<ReflectedClass> classes{};
    std::vector<Enum> enums{};
    struct Capture {
        std::string value{};

        TSNode node{};
    };

    Query class_query(ts_parser_language(parser), CLASS_QUERY);
    Query field_query(ts_parser_language(parser), FIELD_QUERY);
    Query enum_query(ts_parser_language(parser), ENUM_QUERY);
    Query method_query(ts_parser_language(parser), METHOD_QUERY);

    std::set<std::filesystem::path> include_files{};

    auto dir = std::filesystem::path("Fussion/Source");
    for (auto const& entry : std::filesystem::recursive_directory_iterator(dir)) {
        auto path = entry.path();
        if (!entry.is_regular_file() || (path.has_extension() && path.extension() != ".h") || !entry.exists())
            continue;
        if (files_to_ignore.contains(path.filename().string()))
            continue;
        ZoneScopedN("Read File");

        TracyMessage(path.string().c_str(), path.string().size());
        auto file = read_entire_file(path);
        if (!file.has_value()) {
            std::println("[HeaderTool] Failed to open '{}'", path.string());
            continue;
        }

        TSTree* tree;

        {
            ZoneScopedN("Parse String");
            tree = ts_parser_parse_string(parser, nullptr, file->data(), file->size());
        }

        std::unordered_map<std::string_view, ReflectedClass> classes_in_file{};
        std::unordered_map<std::string_view, Enum> enums_in_file{};

        for (auto match : enum_query.execute(ts_tree_root_node(tree))) {
            ZoneScopedN("Enum Query");
            include_files.insert(path);

            std::unordered_map<std::string, Capture> local_captures{};
            for (auto i = 0; i < match.capture_count; i++) {
                auto capture = match.captures[i];
                auto name = std::string(enum_query.capture_name_for_id(capture.index));

                auto start = ts_node_start_byte(capture.node);
                auto end = ts_node_end_byte(capture.node);
                auto value = std::string(*file).substr(start, end - start);

                local_captures[name] = {
                    std::move(value),
                    capture.node,
                };
            }

            auto enum_name = local_captures["name"].value;
            if (!enums_in_file.contains(enum_name)) {
                enums_in_file[enum_name] = Enum{
                    .name = enum_name,
                    .attributes = {},
                    .node = local_captures["enum"].node,
                };
                auto& enum_ = enums_in_file[enum_name];

                for (auto node = ts_node_parent(enum_.node);
                     !ts_node_is_null(node) &&
                     !ts_node_is_error(node) &&
                     !ts_node_is_missing(node);
                     node = ts_node_parent(node)) {

                    bool is_namespace = strcmp(ts_node_type(node), "namespace_definition") == 0;
                    bool is_class = strcmp(ts_node_type(node), "class_specifier") == 0;
                    if (is_namespace || is_class) {
                        auto name_node = ts_node_child_by_field_name(node, "name", 4);
                        auto start = ts_node_start_byte(name_node);
                        auto end = ts_node_end_byte(name_node);

                        auto name = std::string_view(*file).substr(start, end - start);
                        if (enum_.qualified_name.empty()) {
                            enum_.qualified_name = name;
                        } else {
                            enum_.qualified_name = std::format("{}::{}", name, enum_.qualified_name);
                        }
                    }
                }
                enum_.qualified_name += "::" + enum_.name;
            }
            auto& enum_ = enums_in_file[enum_name];

            auto attr_name = local_captures["attr-name"].value;
            if (!enum_.attributes.contains(attr_name))
                enum_.attributes[attr_name] = {};

            enum_.attributes[attr_name].name = attr_name;
            enum_.attributes[attr_name].prefix = local_captures["attr-prefix"].value;

        }

        for (auto match : class_query.execute(ts_tree_root_node(tree))) {
            ZoneScopedN("Class Query");
            include_files.insert(path);

            std::unordered_map<std::string, Capture> local_captures{};
            for (auto i = 0; i < match.capture_count; i++) {
                auto capture = match.captures[i];
                auto name = std::string(class_query.capture_name_for_id(capture.index));

                auto start = ts_node_start_byte(capture.node);
                auto end = ts_node_end_byte(capture.node);
                auto value = std::string(*file).substr(start, end - start);

                local_captures[name] = {
                    std::move(value),
                    capture.node,
                };
            }

            auto class_name = local_captures["class-name"].value;
            if (!classes_in_file.contains(class_name)) {
                classes_in_file[class_name] = ReflectedClass{
                    .name = class_name,
                    .class_node = local_captures["class"].node,
                    .is_component = local_captures.contains("base-class") && local_captures["base-class"].value == "Component",
                };
            }

            auto& klass = classes_in_file[class_name];

            klass.update_attribute(
                local_captures["attr-name"].value,
                local_captures["attr-prefix"].value,
                local_captures["attr-arguments"].value);
        }

        for (auto& klass : classes_in_file | std::views::values) {

            std::unordered_map<std::string, ClassMember> members_in_class{};
            std::unordered_map<std::string, ClassMethod> methods_in_class{};

            for (auto& match : field_query.execute(klass.class_node)) {
                std::unordered_map<std::string, Capture> local_captures{};
                for (auto i = 0; i < match.capture_count; i++) {
                    auto capture = match.captures[i];
                    auto name = std::string(field_query.capture_name_for_id(capture.index));

                    auto start = ts_node_start_byte(capture.node);
                    auto end = ts_node_end_byte(capture.node);
                    auto value = std::string(*file).substr(start, end - start);

                    local_captures[name] = {
                        std::move(value),
                        capture.node,
                    };
                }

                auto const& field_name = local_captures["field-name"].value;
                if (!members_in_class.contains(field_name)) {
                    members_in_class[field_name] = ClassMember{
                        .name = field_name,
                        .node = local_captures["field"].node,
                    };
                }

                auto& member = members_in_class[field_name];
                member.update_attribute(
                    local_captures["attr-name"].value,
                    local_captures["attr-prefix"].value,
                    local_captures["attr-arguments"].value);
            }

            for (auto& match : method_query.execute(klass.class_node)) {
                std::unordered_map<std::string, Capture> local_captures{};
                for (auto i = 0; i < match.capture_count; i++) {
                    auto capture = match.captures[i];
                    auto name = std::string(method_query.capture_name_for_id(capture.index));

                    auto start = ts_node_start_byte(capture.node);
                    auto end = ts_node_end_byte(capture.node);
                    auto value = std::string(*file).substr(start, end - start);

                    local_captures[name] = {
                        std::move(value),
                        capture.node,
                    };
                }

                auto const& field_name = local_captures["method-name"].value;
                if (!methods_in_class.contains(field_name)) {
                    methods_in_class[field_name] = ClassMethod{
                        .name = field_name,
                        .node = local_captures["method"].node,
                    };
                }

                auto& member = methods_in_class[field_name];
                member.update_attribute(
                    local_captures["attr-name"].value,
                    local_captures["attr-prefix"].value,
                    local_captures["attr-arguments"].value);
            }

            std::ranges::transform(methods_in_class, std::back_inserter(klass.methods), [&](auto&& pair) {
                return pair.second;
            });

            std::ranges::transform(members_in_class, std::back_inserter(klass.members), [&](auto&& pair) {
                return pair.second;
            });

            // Figure out the namespace of the class
            for (auto node = ts_node_parent(klass.class_node);
                 !ts_node_is_null(node) &&
                 !ts_node_is_error(node) &&
                 !ts_node_is_missing(node);
                 node = ts_node_parent(node)) {

                bool is_namespace = strcmp(ts_node_type(node), "namespace_definition") == 0;
                bool is_class = strcmp(ts_node_type(node), "class_specifier") == 0;
                if (is_namespace || is_class) {
                    auto name_node = ts_node_child_by_field_name(node, "name", 4);
                    auto start = ts_node_start_byte(name_node);
                    auto end = ts_node_end_byte(name_node);

                    auto name = std::string_view(*file).substr(start, end - start);
                    if (klass.namespace_.empty()) {
                        klass.namespace_ = name;
                    } else {
                        klass.namespace_ = std::format("{}::{}", name, klass.namespace_);
                    }
                }
            }
        }

        std::ranges::transform(classes_in_file, std::back_inserter(classes), [](auto&& pair) {
            return pair.second;
        });

        std::ranges::transform(enums_in_file, std::back_inserter(enums), [](auto&& pair) {
            return pair.second;
        });
    }

    for (auto const& klass : classes) {
        if (klass.has_attribute("Attribute")) {
            if (!klass.name.ends_with("Attribute")) {
                std::println("[HeaderTool][Warning] class/struct '{}' marked with [[Attribute]] but the name does not end in 'Attribute'", klass.name);
            } else {
                attribute_classes[klass.name] = klass;
            }
        }
    }
    std::stringstream ss;

#define TAB "    "
#define TAB_N(x) for(int i = 0; i < x; ++i) { ss << TAB; }
#define NEW_LINE " \n"
#define NEW_LINE_SLASH " \\\n"
#define TAB_NEW_LINE " \t\n"
#define F(s) ss << s << '\n';
#define FMT(s, ...) ss << std::format(s, ##__VA_ARGS__) << '\n';

    F("// GENERATED FILE -- DO NOT EDIT");
    F(R"(#include "Fussion/meta.hpp/meta_all.hpp")");
    F(R"(#include "Fussion/Assets/Asset.h")");
    F(R"(#include "Fussion/Scene/Entity.h")");
    F(R"(#include "Fussion/Math/Color.h")");
    F(R"(#include "Fussion/Reflection/ReflectionRegistry.h")");
    F(R"(#include "Fussion/Reflection/Attributes.h")");

    for (auto const& include : include_files) {
        FMT(R"(#include "{}")", std::filesystem::relative(include, "Fussion/Source").string());
    }
    // @formatter:off
    F("");

    TAB_N(0); F("namespace Fussion {");
    TAB_N(1); F("void register_generated() {");

    TAB_N(2); F("using namespace std::literals;");
    TAB_N(2); F("using namespace Fussion;");
    TAB_N(2); F("namespace meta = meta_hpp;");
    TAB_N(2); F("using meta::metadata_;");
    TAB_N(2); F("using meta::constructor_policy::as_raw_pointer;");
    TAB_N(2); F("using meta::member_policy::as_pointer;");

    F("");
    for (auto const& enum_ : enums) {
        TAB_N(2); FMT("REGISTER_ENUM({}, {});", enum_.qualified_name, enum_.name);
    }

    F("");
    for (auto const& klass : classes) {
        if (!klass.attributes.contains("API"))
            continue;
        TAB_N(2); FMT("meta::class_<{}>(metadata_()", klass.qualified_name());
        TAB_N(3); FMT(R"poo(("Name"s, "{}"s))poo", klass.name);
        TAB_N(3); FMT(")");

        if (klass.is_component) {
            TAB_N(3); FMT(".constructor_<Entity*>(as_raw_pointer)");
        }
        for (auto const& member : klass.members) {
            TAB_N(3); FMT(".member_(\"{}\"s, &{}::{}, as_pointer, metadata_()", member.name, klass.qualified_name(), member.name);
            for (auto const& attr : member.attributes | std::views::values) {
                auto attr_name_with_attribute = attr.name + "Attribute";
                if (attribute_classes.contains(attr_name_with_attribute)) {
                    auto& attr_klass = attribute_classes[attr_name_with_attribute];
                    TAB_N(4); FMT("(\"{}\"s, {}{{{}}})", attr_name_with_attribute, attr_klass.qualified_name(), attr.args);
                }
            }
            TAB_N(3); F(")");
        }

        for (auto const& method : klass.methods) {
            TAB_N(3); FMT(".method_(\"{}\"s, &{}::{}, as_pointer, metadata_()", method.name, klass.qualified_name(), method.name);
            for (auto const& attr : method.attributes | std::views::values) {
                auto attr_name_with_attribute = attr.name + "Attribute";
                if (attribute_classes.contains(attr_name_with_attribute)) {
                    auto& attr_klass = attribute_classes[attr_name_with_attribute];
                    TAB_N(4); FMT("(\"{}\"s, {}{{{}}})", attr_name_with_attribute, attr_klass.qualified_name(), attr.args);
                }
            }
            TAB_N(3); F(")");
        }
        TAB_N(2); FMT(";");
    }

    TAB_N(2); FMT("meta::static_scope_(\"Components\")");
    for (auto const& klass : classes) {
        if (!klass.is_component) continue;
        TAB_N(3); FMT(".typedef_<{}>(\"{}\"s)", klass.qualified_name(), klass.name);
    };
    TAB_N(2); FMT(";");

    TAB_N(1); F("}");
    TAB_N(0); F("}");
    // @formatter:on

    // std::ofstream f(output_path);
    // if (!f.is_open()) {
    //     std::println("[HeaderTool] Failed to open output file");
    //     return -1;
    // }

    std::string output = ss.str();

    if (auto file = read_entire_file(output_path)) {
        if (std::hash<std::string>{}(output) == std::hash<std::string>{}(*file)) {
            std::println("Generated code is the same as a previously generated file. Will not write to avoid rebuild.");
            return 0;
        }
    }

    std::ofstream out_file(output_path, std::ios::trunc);
    out_file << output;
    std::println("[HeaderTool] Done");
}
