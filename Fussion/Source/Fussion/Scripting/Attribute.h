#pragma once

namespace Fussion::Scripting {
    class Attribute {
    public:
        virtual ~Attribute() {}

        virtual std::string to_string() = 0;
    };
}
