#include "../../Vendor/magic_enum/include/magic_enum/magic_enum.hpp"

#include <tree_sitter/api.h>
#include <tree-sitter-cpp.h>
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

constexpr auto ClassQuery = R"scheme(
(class_specifier
  (attribute_declaration
    (attribute
        prefix: (identifier) @attr-prefix
        name: (identifier) @attr-name))
    name: (type_identifier) @class-name
  (base_class_clause
  [
    (type_identifier) @base-class
    (qualified_identifier
        name: (type_identifier) @base-class)
   ])?) @class)scheme";

constexpr auto FieldQuery = R"(
(
 (comment)? @field-comment
 .
 (field_declaration
   (attribute_declaration
     (attribute
       prefix: (identifier) @field-attr-prefix
       name: (identifier) @field-attr-name))
   declarator: (field_identifier) @field-name)
))";

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

std::optional<std::string> ReadEntireFile(std::filesystem::path const& path)
{
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }

    std::ifstream file(path);
    return std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
}

class TSQueryIterator {
public:
    // Constructor
    TSQueryIterator(TSNode const& node, TSQuery const* query)
        : m_Cursor(ts_query_cursor_new()), m_Is_End(false)
    {
        ts_query_cursor_exec(m_Cursor, query, node);
        Advance();
    }

    ~TSQueryIterator()
    {
        if (!m_Is_End) {
            ts_query_cursor_delete(m_Cursor);
        }
    }

    // Iterator traits
    using iterator_category = std::input_iterator_tag;
    using value_type = TSQueryMatch;
    using difference_type = std::ptrdiff_t;
    using pointer = TSQueryMatch*;
    using reference = TSQueryMatch&;

    // Dereference operator
    TSQueryMatch const& operator*() const
    {
        return m_Current_Match;
    }

    // Pointer operator
    TSQueryMatch const* operator->() const
    {
        return &m_Current_Match;
    }

    // Pre-increment
    TSQueryIterator& operator++()
    {
        Advance();
        return *this;
    }

    // Post-increment
    TSQueryIterator operator++(int)
    {
        TSQueryIterator tmp = *this;
        Advance();
        return tmp;
    }

    // Equality comparison
    bool operator==(TSQueryIterator const& other) const
    {
        return m_Is_End == other.m_Is_End;
    }

    // Inequality comparison
    bool operator!=(TSQueryIterator const& other) const
    {
        return !(*this == other);
    }

    // End iterator constructor
    static TSQueryIterator end()
    {
        TSQueryIterator it;
        it.m_Is_End = true;
        return it;
    }

private:
    TSQueryCursor* m_Cursor;
    TSQueryMatch m_Current_Match;
    bool m_Is_End;

    // Private constructor for end iterator
    TSQueryIterator() : m_Cursor(nullptr), m_Is_End(true) {}

    // Advance to the next match
    void Advance()
    {
        if (ts_query_cursor_next_match(m_Cursor, &m_Current_Match)) {
            m_Is_End = false;
        } else {
            m_Is_End = true;
        }
    }
};


class TSQueryResult {
public:
    TSQueryResult(TSNode const& node, TSQuery* query)
        : node(node), query(query) {}

    TSQueryIterator begin() const
    {
        return TSQueryIterator(node, query);
    }

    static TSQueryIterator end()
    {
        return TSQueryIterator::end();
    }

private:
    TSNode node;
    TSQuery* query;
};

class TSQueryWrapper {
public:
    // Constructor to create a TSQuery from a language and source string
    TSQueryWrapper(TSLanguage const* language, std::string const& query_source)
    {
        uint32_t error_offset;
        TSQueryError error_type;

        m_Query = ts_query_new(language, query_source.c_str(), query_source.size(), &error_offset, &error_type);

        if (!m_Query) {
            throw std::runtime_error("Failed to create TSQuery: " + QueryErrorToString(error_type));
        }
    }

    // Destructor
    ~TSQueryWrapper()
    {
        if (m_Query) {
            ts_query_delete(m_Query);
        }
    }

    // Disable copy operations
    TSQueryWrapper(TSQueryWrapper const&) = delete;
    TSQueryWrapper& operator=(TSQueryWrapper const&) = delete;

    // Enable move operations
    TSQueryWrapper(TSQueryWrapper&& other) noexcept : m_Query(other.m_Query)
    {
        other.m_Query = nullptr;
    }

    TSQueryWrapper& operator=(TSQueryWrapper&& other) noexcept
    {
        if (this != &other) {
            if (m_Query) {
                ts_query_delete(m_Query);
            }
            m_Query = other.m_Query;
            other.m_Query = nullptr;
        }
        return *this;
    }

    auto CaptureNameForId(u32 id) const -> std::string_view
    {
        u32 length;
        auto ptr = ts_query_capture_name_for_id(m_Query, id, &length);
        return { ptr, length };
    }

    // Method to create a TSQueryResult from a TSTree
    TSQueryResult Execute(TSNode const& node) const
    {
        return TSQueryResult(node, m_Query);
    }

private:
    TSQuery* m_Query;

    // Helper method to convert TSQueryError to a string
    static std::string QueryErrorToString(TSQueryError error)
    {
        switch (error) {
        case TSQueryErrorNone:
            return "No error";
        case TSQueryErrorSyntax:
            return "Syntax error";
        case TSQueryErrorNodeType:
            return "Invalid node type";
        case TSQueryErrorField:
            return "Invalid field";
        case TSQueryErrorCapture:
            return "Invalid capture";
        case TSQueryErrorStructure:
            return "Invalid structure";
        default:
            return "Unknown error";
        }
    }
};

struct Attribute {
    std::string Namespace{};
    std::string Name{};
};

struct Klass {
    std::string Name{};
    std::string Namespace{};

    std::unordered_map<std::string, Attribute> Attributes{};

    TSNode ClassNode{};
    bool IsComponent{};
};

int main()
{
    auto parser = ts_parser_new();

    ts_parser_set_language(parser, tree_sitter_cpp());

    std::vector<Klass> klasses{};
    struct Capture {
        std::string Value{};
        TSNode Node{};
    };

    auto dir = std::filesystem::path("Fussion/Source");
    for (auto const& entry : std::filesystem::recursive_directory_iterator(dir)) {
        auto path = entry.path();
        if (!entry.is_regular_file() || (path.has_extension() && path.extension() != ".h") || !entry.exists())
            continue;

        auto file = ReadEntireFile(path);
        if (!file.has_value()) {
            std::println("Failed to open '{}'", path.string());
            continue;
        }
        auto tree = ts_parser_parse_string(parser, nullptr, file->data(), file->size());

        TSQueryWrapper class_query(ts_parser_language(parser), ClassQuery);

        std::unordered_map<std::string_view, Klass> classes_in_file{};

        for (auto match : class_query.Execute(ts_tree_root_node(tree))) {
            Klass klass;


            std::unordered_map<std::string, Capture> local_captures{};
            for (auto i = 0; i < match.capture_count; i++) {
                auto capture = match.captures[i];
                auto name = std::string(class_query.CaptureNameForId(capture.index));

                auto start = ts_node_start_byte(capture.node);
                auto end = ts_node_end_byte(capture.node);
                auto value = std::string(*file).substr(start, end - start);

                local_captures[name] = {
                    std::move(value),
                    capture.node,
                };
            }

            using namespace std::string_view_literals;
            auto class_name = local_captures["class-name"].Value;
            if (!classes_in_file.contains(class_name)) {
                classes_in_file[class_name] = Klass {
                    .Name = class_name,
                    .Attributes = {},
                    .ClassNode = local_captures["class"].Node,
                    .IsComponent = local_captures.contains("base-class") && local_captures["base-class"].Value == "Component",
                };
            }

            auto attr_name = local_captures["attr-name"].Value;
            if (!classes_in_file[class_name].Attributes.contains(attr_name))
                classes_in_file[class_name].Attributes[attr_name] = {};

            classes_in_file[class_name].Attributes[attr_name].Name = attr_name;
            classes_in_file[class_name].Attributes[attr_name].Namespace = local_captures["attr-prefix"].Value;

            // std::println("Attributes for class '{}'", klass.Name);
            // for (auto const& [name, attribute] : klass.Attributes) {
            //     (void)name;
            //     std::println("{}", attribute.Name);
            // }
        }

        // Figure out the namespace of the class
        for (auto& klass : classes_in_file | std::views::values) {
            TSQueryWrapper field_query(ts_parser_language(parser), ClassQuery);
            for (auto& match : field_query.Execute(klass.ClassNode)) {
                std::unordered_map<std::string, Capture> local_captures{};
                for (auto i = 0; i < match.capture_count; i++) {
                    auto capture = match.captures[i];
                    auto name = std::string(class_query.CaptureNameForId(capture.index));

                    auto start = ts_node_start_byte(capture.node);
                    auto end = ts_node_end_byte(capture.node);
                    auto value = std::string(*file).substr(start, end - start);

                    local_captures[name] = {
                        std::move(value),
                        capture.node,
                    };
                }
            }


            for(auto node = ts_node_parent(klass.ClassNode);
                !ts_node_is_null(node) &&
                !ts_node_is_error(node) &&
                !ts_node_is_missing(node);
                node = ts_node_parent(node)) {

                if (strcmp(ts_node_type(node), "namespace_definition") == 0) {
                    auto name_node = ts_node_named_child(node, 0);
                    auto start = ts_node_start_byte(name_node);
                    auto end = ts_node_end_byte(name_node);

                    auto name = std::string_view(*file).substr(start, end - start);
                    if (klass.Namespace.empty()) {
                        klass.Namespace = name;
                    } else {
                        klass.Namespace = std::format("{}::{}", name, klass.Namespace);
                    }
                }
            }
        }

        for (auto const& klass : classes_in_file | std::views::values) {
            std::println("Class {} :: {} | Is Component: {}", klass.Namespace, klass.Name, klass.IsComponent);
            for (auto const& attribute : klass.Attributes | std::views::values) {
                std::println("\t{}::{}", attribute.Namespace, attribute.Name);
            }
            std::println("");
        }
    }

//     auto file = ReadEntireFile("Fussion/Source/Fussion/Scene/Components/Camera.h");
//     if (!file.has_value()) {
//         std::println("Cannot read file");
//         return 1;
//     }
//
//     auto tree = ts_parser_parse_string(parser, nullptr, file->data(), file->size());
//
//     auto s_expr = std::string();
//
//     TSQueryWrapper my_query(ts_parser_language(parser), s_expr);
//
//     Klass klass{};
//
//     struct ClassCapture {
//         TSNode Node{};
//         std::string_view Name{};
//     };
//
//     std::unordered_map<std::string_view, ClassCapture> captures;
//
//     for (auto match : my_query.Execute(ts_tree_root_node(tree))) {
//         for (auto i = 0; i < match.capture_count; i++) {
//             auto capture = match.captures[i];
//             auto name = my_query.CaptureNameForId(capture.index);
//
//             auto start = ts_node_start_byte(capture.node);
//             auto end = ts_node_end_byte(capture.node);
//             captures[name] = {
//                 .Node = capture.node,
//                 .Name = std::string_view(*file).substr(start, end - start)
//             };
//         }
//     }
//
//     klass.Name = captures["class-name"].Name;
//
//     TSQueryWrapper fields_query(ts_parser_language(parser), R"(
// (
//  (comment)? @field-comment
//  .
//  (field_declaration
//    (attribute_declaration
//      (attribute
//        prefix: (identifier) @field-attr-prefix
//        name: (identifier) @field-attr-name (#eq? @field-attr-name "Export")))
//    declarator: (field_identifier) @field-name)
// ))");
//
//     struct Prop {
//         std::string Name{};
//         std::optional<std::string> Comment{};
//     };
//
//     std::vector<Prop> props{};
//
//     for (auto& match : fields_query.Execute(captures["class"].Node)) {
//         std::unordered_map<std::string_view, std::string_view> metadata{};
//
//         for (auto i = 0; i < match.capture_count; i++) {
//             auto capture = match.captures[i];
//             auto name = fields_query.CaptureNameForId(capture.index);
//
//             auto start = ts_node_start_byte(capture.node);
//             auto end = ts_node_end_byte(capture.node);
//             auto value = std::string_view(*file).substr(start, end - start);
//             metadata[name] = value;
//         }
//
//         if (metadata["field-attr-prefix"] == "Fussion" && metadata["field-attr-name"] == "Export") {
//             props.push_back(
//                 Prop{
//                     std::string(metadata["field-name"]),
//                     metadata.contains("field-comment") ? std::optional<std::string>(metadata["field-comment"]) : std::nullopt,
//                 }
//                 );
//         }
//
//     }
//
//     for (auto const& prop : props) {
//         std::println("{} {}", prop.Name, prop.Comment.value_or("null"));
    // }
}
