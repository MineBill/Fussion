#include "FussionPCH.h"
#include "Args.h"

namespace Fussion {

int g_Argc{};
char** g_Argv{};
std::string g_SingleLine{};

void Args::Collect(int argc, char** argv)
{
    g_Argc = argc;
    g_Argv = argv;

    std::stringstream arguments{};
    for (int i = 1; i < argc; i++) {
        arguments << argv[i] << ' ';
    }
    g_SingleLine = arguments.str();
}

int Args::Argc()
{
    return g_Argc;
}

char** Args::Argv()
{
    return g_Argv;
}

std::string const& Args::AsSingleLine()
{
    return g_SingleLine;
}

}
