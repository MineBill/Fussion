#include "FussionPCH.h"
#include "Args.h"

namespace Fussion {

int g_Argc{};
char** g_Argv{};
std::string g_SingleLine{};

void Args::collect(int argc, char** argv)
{
    g_Argc = argc;
    g_Argv = argv;

    std::stringstream arguments{};
    for (int i = 1; i < argc; i++) {
        arguments << argv[i] << ' ';
    }
    g_SingleLine = arguments.str();
}

int Args::argc()
{
    return g_Argc;
}

char** Args::argv()
{
    return g_Argv;
}

std::string const& Args::as_single_line()
{
    return g_SingleLine;
}

}
