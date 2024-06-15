#pragma once

#include <sstream>

namespace Fussion
{
    class ISerializable
    {
    public:

        void Serialize(std::stringstream& stream);
    };
}