#pragma once
#include <filesystem>

#include "EditorAssetManager.h"
#include "Engin5/Core/Types.h"

class Project
{
public:
    static Project& ActiveProject() {
        static Project project;
        return project;
    }

    static Ref<Project> Load(std::filesystem::path path);

    Ref<EditorAssetManager>& GetAssetManager() { return m_AssetManager; }

private:
    Ref<EditorAssetManager> m_AssetManager{};
};
