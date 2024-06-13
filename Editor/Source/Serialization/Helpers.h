#pragma once
#include "Engin5/Core/Types.h"
#include "kdlpp.h"

static auto FindNode(std::vector<kdl::Node> const& nodes, std::string_view name) -> std::optional<kdl::Node>
{
    auto const pos = std::ranges::find_if(nodes, [&name](auto const& node) {
        return node.name() == name;
    });
    if (pos != nodes.end()) {
        return *pos;
    }
    return std::nullopt;
}

template<typename T>
static auto GetArgAt(kdl::Node const& node, u32 index, kdl::Type expected_type) -> std::optional<T>
{
    auto const args = node.args();
    if (index >= args.size()) {
        return std::nullopt;
    }

    if (args[index].type() != expected_type) {
        return std::nullopt;
    }

    return args[index].as<T>();
}

template<typename T>
static auto GetProperty(kdl::Node const& node, std::string const& key) -> std::optional<T>
{
    if (node.properties().contains(key)) {
        return node.properties().at(key).as<T>();
    }
    return std::nullopt;
}