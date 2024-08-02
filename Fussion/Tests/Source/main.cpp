#include "Fussion/Log/Log.h"

#include <catch2/catch_session.hpp>

int main(int argc, char* argv[])
{
    Fussion::Log::DefaultLogger()->SetLogLevel(Fussion::LogLevel::Debug);
    int result = Catch::Session().run(argc, argv);
    return result;
}
