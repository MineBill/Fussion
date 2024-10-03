#pragma once

namespace Fussion {
    class Args {
    public:
        static void Collect(int argc, char** argv);

        static int Argc();
        static char** Argv();

        static std::string const& AsSingleLine();
    };
}
