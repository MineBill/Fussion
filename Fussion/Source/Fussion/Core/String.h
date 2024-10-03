#pragma once
#include <Fussion/Core/Maybe.h>
#include <Fussion/Core/Mem.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Log/Formatters.h>

#define HAS_MICROSHIT_FINALLY_IMPLEMENTED_MULTIDIMENSIONAL_SUBSCRIPT 0

namespace Fussion {
    namespace strutils {
        size_t strlen(char const* str);
    }

    struct String {
        Slice<char> data {};

        String() = default;
        String(char const* cstr)
            : data(const_cast<char*>(cstr), CAST(u32, strutils::strlen(cstr)))
        { }
        explicit String(Slice<char> buffer)
            : data(std::move(buffer))
        { }

        String Clone(Mem::Allocator const& allocator = Mem::GetHeapAllocator()) const;
        void Free(Mem::Allocator const& allocator);

        /// Splits the string using separator, returning an allocated slice of all the parts.
        /// The parts are views into the original string.
        Slice<String> Split(String const& separator, Mem::Allocator const& allocator = Mem::GetHeapAllocator()) const;

        /// Returns a new allocated string with 'old_str' replaced by 'new_str'.
        /// If the string is empty, an empty Maybe is returned.
        /// If 'old_str' could not be located, an empty Maybe is returned.
        Maybe<String> Replace(String const& old_str, String const& new_str, Mem::Allocator const& allocator = Mem::GetHeapAllocator()) const;

        /// Returns the index of the needle if found, None otherwise.
        Maybe<usz> IndexOf(String const& needle) const;

        usz Len() const;

        /// Returns a sub-view into this string.
        /// @param start is the starting index.
        /// @param end is the exclusive end index.
        String View(usz start, usz end) const;

        /// Trims whitespace both from left and right;
        void Trim(String whitespace = " ");

        void TrimLeft(String whitespace = " ");
        void TrimRight(String whitespace = " ");

        bool operator==(String const&) const;
        char& operator[](usz) const;
#if HAS_MICROSHIT_FINALLY_IMPLEMENTED_MULTIDIMENSIONAL_SUBSCRIPT
        String operator[](int, int) const;
#endif

        /// Create an allocated string form a cstring.
        static String Alloc(char const* str, Mem::Allocator const& allocator = Mem::GetHeapAllocator());

        template<typename... Args>
        static String Format(String fmt, Args&&...)
        {
            (void)fmt;
            PANIC("TODO");
        }
    };
}

// TODO: Figure out a way to make this formattable without using std::string_view.
FSN_MAKE_FORMATTABLE(Fussion::String, "{}", std::string_view(v.data.ptr, v.data.length))

#ifdef FSN_CORE_USE_GLOBALLY
using Fussion::String;
#endif
