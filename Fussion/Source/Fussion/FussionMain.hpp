#pragma once
#include <Fussion/OS/Args.h>
#include <Fussion/Reflection/ReflectionRegistry.h>

void FussionMain();

int main(int argc, char** argv)
{
    Fussion::ReflectionRegistry::Register();
    Fussion::Args::Collect(argc, argv);

    FussionMain();
}
