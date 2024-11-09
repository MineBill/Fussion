#pragma once

#include <Fussion/Core/Types.h>
#include <Fussion/Scripting/Attribute.h>

namespace Fussion::Scripting {
    class EditableAttribute final : public Attribute {
    public:
        virtual std::string to_string() override
        {
            return "EditableAttribute";
        }
    };
}
