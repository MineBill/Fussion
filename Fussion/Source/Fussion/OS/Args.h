#pragma once

namespace Fussion {

class Args {
public:
    static void collect(int argc, char** argv);

    static int argc();
    static char** argv();

    static std::string const& as_single_line();
    // static std::vector<std::string> const& AsVector();
};

}
