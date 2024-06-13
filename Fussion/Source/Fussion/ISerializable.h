#pragma once
#include "Reflect/Reflect.h"
#include "Generated/ISerializable_reflect_generated.h"

#include <sstream>

namespace Fussion
{
    REFLECT_CLASS()
    class ISerializable: REFLECT_BASE
    {
        REFLECT_GENERATED_BODY()
    public:

        void Serialize(std::stringstream& stream);
    };
}