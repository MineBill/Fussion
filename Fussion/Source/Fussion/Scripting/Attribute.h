#pragma once

namespace Fussion::Scripting {
class Attribute {
public:
    virtual ~Attribute() {}

    virtual std::string ToString() = 0;
};

}
