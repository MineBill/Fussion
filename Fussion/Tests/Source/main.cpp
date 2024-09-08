#include "Fussion/Log/Log.h"

#include <catch2/catch_session.hpp>

int main(int argc, char* argv[])
{
    Fussion::Log::default_logger()->set_log_level(Fussion::LogLevel::Debug);
    int result = Catch::Session().run(argc, argv);
    return result;
}
