﻿#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Layer.h"

struct EditorProject {
    std::string Name {};
    std::filesystem::path Location {};
};

class ProjectCreatorLayer final : public Fussion::Layer {
public:
    virtual void OnStart() override;
    virtual void OnUpdate(f32 delta) override;
    virtual void OnEvent(Fussion::Event& event) override;

    void Serialize(Fussion::Serializer& s) const;
    void Deserialize(Fussion::Deserializer& ds);

    void AddProject(std::filesystem::path const& path);
    void SaveProjects() const;
    void LoadProjects();

private:
    bool m_OpenNewProjectPopup {};
    bool m_OpenImportProjectPopup {};
    bool m_ProjectNameValidated { false };

    std::vector<EditorProject> m_Projects {};
};
