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

    String String::Clone(Mem::Allocator const& allocator) const
    {
        String s;
        s.data = Mem::Alloc<char>(data.length, allocator);

        Mem::Copy(s.data, data);
        return s;
    }

    void String::Free(Mem::Allocator const& allocator)
    {
        if (data.length == 0) {
            return;
        }
        Mem::Free(data.ptr, allocator);
        data.ptr = nullptr;
        data.length = 0;
    }

    Slice<String> String::Split(String const& separator, Mem::Allocator const& allocator) const
    {
        if (separator == *this) {
            // TODO: big oof
            auto part = Mem::Alloc<String>(1, allocator);
            part[0] = *this;
            return part;
        }
        String str = *this;
        u32 count = 1;
        while (true) {
            auto pos = str.IndexOf(separator);
            if (pos.IsEmpty())
                break;
            str = str.View(*pos + separator.Len(), str.Len());
            count++;
        }

        if (count == 1) {
            return {};
        }

        auto parts = Mem::Alloc<String>(count, allocator);

        str = *this;
        count = 0;
        while (true) {
            auto pos = str.IndexOf(separator);
            if (pos.IsEmpty()) {
                parts[count] = str.View(0, str.Len());
                break;
            }
            parts[count] = str.View(0, *pos);
            str = str.View(*pos + separator.Len(), str.Len());
            count++;
        }
        return parts;
    }

    Maybe<String> String::Replace(String const& old_str, String const& new_str, Mem::Allocator const& allocator) const
    {
        if (Len() == 0 || old_str.Len() == 0) {
            return None();
        }

        auto parts = Split(old_str, Mem::GetTempAllocator());
        if (parts.len() == 0) {
            return None();
        }

        // only one part means old_str == *this
        if (parts.len() == 1) {
            return new_str.Clone(allocator);
        }
        auto new_size = Len() - old_str.Len() * (parts.len() - 1) + new_str.Len() * (parts.len() - 1);
        auto str_buffer = Mem::Alloc<char>(new_size, allocator);
        usz pos = 0;
        for (auto const& part : parts) {
            // str_buffer.append()
            Mem::Copy(str_buffer.SubSlice(pos, 1000), part.data);
            pos += part.Len();
            Mem::Copy(str_buffer.SubSlice(pos, 1000), new_str.data);
            pos += new_str.Len();
        }

        return String(str_buffer);
    }

    Maybe<usz> String::IndexOf(String const& needle) const
    {
        for (usz i = 0; i < data.length; ++i) {
            if (View(i, i + needle.data.length) == needle) {
                return i;
            }
        }
        return None();
    }

    usz String::Len() const
    {
        return data.length;
    }

    String String::View(usz start, usz end) const
    {
        VERIFY(end >= start);
        String s;
        s.data.ptr = data.ptr + start;
        s.data.length = end - start;
        return s;
    }

    void String::Trim(String whitespace)
    {
        TrimLeft(whitespace);
        TrimRight(whitespace);
    }

    void String::TrimLeft(String whitespace)
    {
        if (Len() == 0)
            return;
        auto match_any = [](char ch, String from) {
            for (usz i = 0; i < from.Len(); ++i) {
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

        *this = View(j, Len());
    }

    void String::TrimRight(String whitespace)
    {
        if (Len() == 0)
            return;
        auto match_any = [](char ch, String from) {
            for (usz i = 0; i < from.Len(); ++i) {
                if (from[i] == ch)
                    return true;
            }
            return false;
        };
        s64 j = Len() - 1;
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
        *this = View(0, j);
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

    String String::Alloc(char const* str, Mem::Allocator const& allocator)
    {
        String s(str);
        return s.Clone(allocator);
    }
}
