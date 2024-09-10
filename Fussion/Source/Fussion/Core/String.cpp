#include "FussionPCH.h"
#include "String.h"

#include <cstring>

namespace Fussion {

    size_t strutils::strlen(char const* str)
    {
        size_t len = 0;
        while (str[len] != '\0')
            ++len;
        return len;
    }

    String String::clone(mem::Allocator const& allocator) const
    {
        String s;
        s.data = mem::alloc<char>(data.length, allocator);

        mem::copy(s.data, data);
        return s;
    }

    void String::free()
    {
        if (data.length == 0) {
            return;
        }
        mem::free(data.ptr);
        data.ptr = nullptr;
    }

    Slice<String> String::split(String separator, mem::Allocator const& allocator) const
    {
        String str = *this;
        u32 count = 1;
        while (true) {
            auto pos = str.index_of(separator);
            if (pos.is_empty())
                break;
            str = str.view(*pos + separator.len(), str.len());
            count++;
        }

        auto parts = mem::alloc<String>(count, allocator);

        str = *this;
        count = 0;
        while (true) {
            auto pos = str.index_of(separator);
            if (pos.is_empty()) {
                parts[count] = str.view(0, str.len());
                break;
            }
            parts[count] = str.view(0, *pos);
            str = str.view(*pos + separator.len(), str.len());
            count++;
        }
        return parts;
    }
    
    void String::replace(String from, String to, mem::Allocator const& allocator)
    {
        // auto parts = split(from, allocator);
        // StringBuilder sb(allocator);
        // for (usz i = 0; i < parts.length; ++i) {
        //     sb.append(parts[i]);
        //     sb.append(to);
        // }
    }

    Maybe<usz> String::index_of(String needle) const
    {
        for (usz i = 0; i < data.length; ++i) {
            if (view(i, i + needle.data.length) == needle) {
                return i;
            }
        }
        return None();
    }

    usz String::len() const
    {
        return data.length;
    }

    String String::view(usz start, usz end) const
    {
        VERIFY(end >= start);
        String s;
        s.data.ptr = data.ptr + start;
        s.data.length = end - start;
        return s;
    }

    void String::trim(String whitespace) {
        trim_left(whitespace);
        trim_right(whitespace);
    }

    void String::trim_left(String whitespace)
    {
        if (len() == 0) return;
        auto match_any = [](char ch, String from) {
            for (usz i = 0; i < from.len(); ++i) {
                if (from[i] == ch)
                    return true;
            }
            return false;
        };
        usz j = 0;
        while (j < data.length) {
            if (!match_any(data[j], whitespace))
                break;
            j++;
        }

        if (j == data.length) {
            data.length = 0;
            return;
        }

        *this = view(j, len());
    }

    void String::trim_right(String whitespace)
    {
        if (len() == 0) return;
        auto match_any = [](char ch, String from) {
            for (usz i = 0; i < from.len(); ++i) {
                if (from[i] == ch)
                    return true;
            }
            return false;
        };
        s64 j = len() - 1;
        while (j >= 0) {
            if (!match_any(data[j], whitespace)) {
                j++;
                break;
            }
            j--;
        }

        if (j <= 0) {
            data.length = 0;
            return;
        }
        *this = view(0, j);
    }

    bool String::operator==(String const& other) const
    {
        if (other.data.length != data.length)
            return false;
        if ((other.data.length == data.length) && (data.length == 0))
            return true;
        return std::memcmp(other.data.ptr, data.ptr, data.length) == 0;
    }

    char& String::operator[](int index) const
    {
        VERIFY(index >= 0, "Negative index");
        VERIFY(index < data.length, "Index out-of-bounds");
        return data[index];
    }

#if HAS_MICROSHIT_FINALLY_IMPLEMENTED_MULTIDIMENSIONAL_SUBSCRIPT
    String String::operator[](int start, int end) const
    {
        String s;
        s.data.ptr = data.ptr + start;
        s.data.length = end - start;
        return s;
    }
#endif

    String String::alloc(char const* str, mem::Allocator const& allocator)
    {
        String s(str);
        return s.clone(allocator);
    }
}
