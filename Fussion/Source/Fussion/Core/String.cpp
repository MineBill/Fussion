#include "FussionPCH.h"
#include "String.h"
#include "Input/Keys.h"

#include <cstring>

namespace Fussion {

    size_t strutils::strlen(char const* str)
    {
        size_t len = 0;
        while (str[len] != '\0')
            ++len;
        return len;
    }

    String String::clone(Mem::Allocator const& allocator) const
    {
        String s;
        s.data = Mem::alloc<char>(data.length, allocator);

        Mem::copy(s.data, data);
        return s;
    }

    void String::free(Mem::Allocator const& allocator)
    {
        if (data.length == 0) {
            return;
        }
        Mem::free(data.ptr, allocator);
        data.ptr = nullptr;
        data.length = 0;
    }

    Slice<String> String::split(String const& separator, Mem::Allocator const& allocator) const
    {
        if (separator == *this) {
            // TODO: big oof
            auto part = Mem::alloc<String>(1, allocator);
            part[0] = *this;
            return part;
        }
        String str = *this;
        u32 count = 1;
        while (true) {
            auto pos = str.index_of(separator);
            if (pos.is_empty())
                break;
            str = str.view(*pos + separator.size(), str.size());
            count++;
        }

        if (count == 1) {
            return {};
        }

        auto parts = Mem::alloc<String>(count, allocator);

        str = *this;
        count = 0;
        while (true) {
            auto pos = str.index_of(separator);
            if (pos.is_empty()) {
                parts[count] = str.view(0, str.size());
                break;
            }
            parts[count] = str.view(0, *pos);
            str = str.view(*pos + separator.size(), str.size());
            count++;
        }
        return parts;
    }

    Maybe<String> String::replace(String const& old_str, String const& new_str, Mem::Allocator const& allocator) const
    {
        if (size() == 0 || old_str.size() == 0) {
            return None();
        }

        auto parts = split(old_str, Mem::GetTempAllocator());
        if (parts.len() == 0) {
            return None();
        }

        // only one part means old_str == *this
        if (parts.len() == 1) {
            return new_str.clone(allocator);
        }
        auto new_size = size() - old_str.size() * (parts.len() - 1) + new_str.size() * (parts.len() - 1);
        auto str_buffer = Mem::alloc<char>(new_size, allocator);
        usz pos = 0;
        for (auto const& part : parts) {
            // str_buffer.append()
            Mem::copy(str_buffer.SubSlice(pos, 1000), part.data);
            pos += part.size();
            Mem::copy(str_buffer.SubSlice(pos, 1000), new_str.data);
            pos += new_str.size();
        }

        return String(str_buffer);
    }

    Maybe<usz> String::index_of(String const& needle) const
    {
        for (usz i = 0; i < data.length; ++i) {
            if (view(i, i + needle.data.length) == needle) {
                return i;
            }
        }
        return None();
    }

    usz String::size() const
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

    void String::trim(String whitespace)
    {
        trim_left(whitespace);
        trim_right(whitespace);
    }

    void String::trim_left(String whitespace)
    {
        if (size() == 0)
            return;
        auto match_any = [](char ch, String from) {
            for (usz i = 0; i < from.size(); ++i) {
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

        *this = view(j, size());
    }

    void String::trim_right(String whitespace)
    {
        if (size() == 0)
            return;
        auto match_any = [](char ch, String from) {
            for (usz i = 0; i < from.size(); ++i) {
                if (from[i] == ch)
                    return true;
            }
            return false;
        };
        s64 j = size() - 1;
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

    char& String::operator[](usz index) const
    {
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

    String String::alloc(char const* str, Mem::Allocator const& allocator)
    {
        String s(str);
        return s.clone(allocator);
    }
}
