#pragma once
#include <Fussion/Core/Types.h>
#include <Fussion/Core/Maybe.h>
#include <Fussion/Core/Mem.h>
#include <FUssion/Log/Formatters.h>

#define HAS_MICROSHIT_FINALLY_IMPLEMENTED_MULTIDIMENSIONAL_SUBSCRIPT 0

namespace Fussion {
    namespace strutils {
        size_t strlen(char const* str);
    }

    struct String {
        Slice<char> data{};

        String() = default;
        String(char const* cstr): data(const_cast<char*>(cstr), CAST(u32, strutils::strlen(cstr))) {}

        String clone(mem::Allocator const& allocator = mem::heap_allocator()) const;
        void free();

        /// Splits the string using separator, returning an allocated slice of all the parts.
        /// The parts are views into the original string.
        Slice<String> split(String separator, mem::Allocator const& allocator = mem::heap_allocator()) const;
        
        void replace(String from, String to, mem::Allocator const& allocator = mem::heap_allocator());

        /// Returns the index of the needle if found, None otherwise.
        Maybe<usz> index_of(String needle) const;

        usz len() const;

        /// Returns a sub-view into this string.
        /// @param start is the starting index.
        /// @param end is the exclusive end index.
        String view(usz start, usz end) const;

        /// Trims whitespace both from left and right;
        void trim(String whitespace = " ");

        void trim_left(String whitespace = " ");
        void trim_right(String whitespace = " ");

        bool operator==(String const&) const;
        char& operator[](int) const;
#if HAS_MICROSHIT_FINALLY_IMPLEMENTED_MULTIDIMENSIONAL_SUBSCRIPT
        String operator[](int, int) const;
#endif

        static String alloc(char const* str, mem::Allocator const& allocator = mem::heap_allocator());

        template<typename... Args>
        static String format(String fmt, Args&&...) {}
    };
}

// TODO: Figure out a way to make this formattable without using std::string_view.
FSN_MAKE_FORMATTABLE(Fussion::String, "{}", std::string_view(v.data.ptr, v.data.length))