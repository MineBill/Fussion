#pragma once
#include "Common.h"

#include <tree_sitter/api.h>
#include <tree-sitter-cpp.h>

#include <tracy/Tracy.hpp>

class QueryIterator {
public:
    QueryIterator(TSNode const& node, TSQuery const* query);
    ~QueryIterator();

    using iterator_category = std::input_iterator_tag;
    using value_type = TSQueryMatch;
    using difference_type = std::ptrdiff_t;
    using pointer = TSQueryMatch*;
    using reference = TSQueryMatch&;

    TSQueryMatch const& operator*() const
    {
        return m_current_match;
    }

    TSQueryMatch const* operator->() const
    {
        return &m_current_match;
    }

    QueryIterator& operator++()
    {
        advance();
        return *this;
    }

    QueryIterator operator++(int)
    {
        QueryIterator tmp = *this;
        advance();
        return tmp;
    }

    bool operator==(QueryIterator const& other) const
    {
        return m_is_end == other.m_is_end;
    }

    bool operator!=(QueryIterator const& other) const
    {
        return !(*this == other);
    }

    static QueryIterator end()
    {
        QueryIterator it;
        it.m_is_end = true;
        return it;
    }

private:
    TSQueryCursor* m_cursor;
    TSQueryMatch m_current_match;
    bool m_is_end;

    // Private constructor for end iterator
    QueryIterator() : m_cursor(nullptr), m_is_end(true) {}

    // Advance to the next match
    void advance();
};

class QueryResult {
public:
    QueryResult(TSNode const& node, TSQuery* query)
        : m_node(node), m_query(query) {}

    [[nodiscard]]
    QueryIterator begin() const
    {
        return QueryIterator(m_node, m_query);
    }

    static QueryIterator end()
    {
        return QueryIterator::end();
    }

private:
    TSNode m_node;
    TSQuery* m_query;
};

class Query {
public:
    Query(TSLanguage const* language, std::string const& query_source);
    ~Query();

    Query(Query const&) = delete;
    Query& operator=(Query const&) = delete;
    Query(Query&& other) noexcept;
    Query& operator=(Query&& other) noexcept;

    QueryResult execute(TSNode const& node) const;

    auto capture_name_for_id(u32 id) const -> std::string_view;

private:
    TSQuery* m_query;

    // Helper method to convert TSQueryError to a string
    static std::string query_error_to_string(TSQueryError error);
};
