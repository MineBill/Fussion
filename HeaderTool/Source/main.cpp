#include <filesystem>
#include <print>
#include <unordered_map>

#include <Reflect/Reflect.h>

const std::string CachePath = ".reflect_cache";

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::print("[ERROR] No arguments provided.\n");
        exit(1);
    }

    std::print("Running code generation for {}\n", argv[1]);


    Reflect::Parser::Cache::Open(std::filesystem::current_path() / CachePath);

    Reflect::ReflectAdditionalOptions options = { };

    std::vector<std::string> directories;
    for (size_t i = 0; i < argc; ++i)
    {
        bool foundPath = false;
        try
        {
            std::filesystem::path absPath = std::filesystem::absolute(argv[i]);
            if (std::filesystem::is_directory(absPath))
            {
                directories.push_back(absPath.string());
                foundPath = true;
            }
        }
        catch (std::error_code)
        { }

        if (!foundPath)
        {
            std::string arg = argv[i];
            std::string argKey = arg.substr(0, arg.find('='));
            std::unordered_map<const std::string, std::string>::iterator itr = options.options.find(argKey);
            if (itr != options.options.end())
            {
                itr->second = arg.substr(arg.find('=') + 1);
            }
        }
    }

    if (!directories.empty())
    {
        Reflect::CodeGeneration::CodeGenerate codeGenerate;
        Reflect::Parser::FileParser parser;
        if (std::ifstream i_file(Reflect::Keys::ReflectIgnoreStringsFileName); i_file.is_open())
        {
            i_file.seekg(0, std::ifstream::end);
            size_t size = i_file.tellg();
            i_file.seekg(0, std::ifstream::beg);

            std::string data;
            data.resize(size);
            i_file.read(data.data(), size);
            i_file.close();
            parser.SetIgnoreStrings(Reflect::Util::SplitString(data, '\n'));
        }

        for (auto& dir : directories)
        {
            parser.ParseDirectory(dir, &options);
        }
        codeGenerate.Reflect(parser, &options);
    }

    Reflect::Parser::Cache::Save(std::filesystem::current_path() / CachePath);

    return 0;
}