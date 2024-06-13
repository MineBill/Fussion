#include "angelscript.h"
#include "scriptstdstring/scriptstdstring.h"
#include "scriptbuilder/scriptbuilder.h"
#include "AngelDumper.h"
#include "Engin5/Core/Core.h"

#include <print>
#include <fstream>

void MessageCallback(const asSMessageInfo *msg, void *param)
{
    (void)param;
    auto type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";
    std::println("{} ({}, {}) : {} : {}", msg->section, msg->row, msg->col, type, msg->message);
}

void Print(std::string& msg)
{
    std::println("{}", msg);
}

int main()
{
    std::println("Angelscript Test!");

    asIScriptEngine* engine = asCreateScriptEngine();
    auto r = engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);
    EASSERT(r >= 0, "Failed to set message callback");

    RegisterStdString(engine);
    // RegisterStdStringUtils(engine);

    r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
    EASSERT(r >= 0, "Failed to register function");

    engine->RegisterTypedef("f32", "float");
    engine->RegisterTypedef("f64", "double");

    CScriptBuilder builder;

    r = builder.StartNewModule(engine, "NewModule");
    EASSERT(r >= 0, "Error while starting new module");

    r = builder.AddSectionFromFile("Assets/Scripts/HelloWorld.as");
    EASSERT(r >= 0, "Error while adding section from file");

    r = builder.BuildModule();
    EASSERT(r >= 0, "Error while building module");

    auto module = engine->GetModule("NewModule");

    auto func = module->GetFunctionByDecl("void Main()");
    EASSERT(func, "'Main' function not found");

    auto metadata = builder.GetMetadataForFunc(module->GetFunctionByName("DoSomething"));
    for (auto const& meta : metadata)
    {
        std::println("{}", meta);
    }

    auto ctx = engine->CreateContext();
    ctx->Prepare(func);
    ctx->Execute();

    AngelDumper dumper(engine);
    auto dump = dumper.Dump();

    // Write to file named 'as.predefined'
    std::ofstream file("as.predefined");
    file << dump.str();
}
