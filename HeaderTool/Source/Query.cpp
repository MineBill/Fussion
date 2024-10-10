#include "Query.h"

#include <stdexcept>

QueryIterator::QueryIterator(TSNode const& node, TSQuery const* query): m_cursor(ts_query_cursor_new()), m_is_end(false) {
    ts_query_cursor_exec(m_cursor, query, node);
    advance();
}

QueryIterator::~QueryIterator() {
    if (!m_is_end) {
        ts_query_cursor_delete(m_cursor);
    }
}

void QueryIterator::advance() {
    if (ts_query_cursor_next_match(m_cursor, &m_current_match)) {
        m_is_end = false;
    } else {
        m_is_end = true;
    }
}

Query::Query(TSLanguage const* language, std::string const& query_source) {
    ZoneScoped;
    u32 error_offset;
    TSQueryError error_type;

    {
        ZoneScopedN("ts_query_new");
        m_query = ts_query_new(language, query_source.c_str(), query_source.size(), &error_offset, &error_type);
    }

    if (!m_query) {
        throw std::runtime_error("Failed to create TSQuery: " + query_error_to_string(error_type));
    }
}

Query::~Query() {
    ZoneScoped;
    if (m_query) {
        ts_query_delete(m_query);
    }
}

Query::Query(Query&& other) noexcept: m_query(other.m_query) {
    ZoneScoped;
    other.m_query = nullptr;
}

Query& Query::operator=(Query&& other) noexcept {
    ZoneScoped;
    if (this != &other) {
        if (m_query) {
            ts_query_delete(m_query);
        }
        m_query = other.m_query;
        other.m_query = nullptr;
    }
    return *this;
}

auto Query::capture_name_for_id(u32 id) const -> std::string_view {
    ZoneScoped;
    u32 length;
    auto ptr = ts_query_capture_name_for_id(m_query, id, &length);
    return { ptr, length };
}

QueryResult Query::execute(TSNode const& node) const {
    ZoneScoped;
    return QueryResult(node, m_query);
}

std::string Query::query_error_to_string(TSQueryError error) {
    ZoneScoped;
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
