#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Layer.h"

struct EditorProject {
    std::string Name{};
    std::filesystem::path Location{};
};

class ProjectCreatorLayer final : public Fussion::Layer {
public:
    virtual void on_start() override;
    virtual void on_update(f32 delta) override;
    virtual void on_event(Fussion::Event& event) override;

    void Serialize(Fussion::Serializer& s) const;
    void Deserialize(Fussion::Deserializer& ds);

    void AddProject(std::filesystem::path const& path);
    void SaveProjects() const;
    void LoadProjects();
private:
    Vector2 m_MouseDragStarPos{}, m_Offset{};
    bool m_StartedDragging{};

    bool m_OpenNewProjectPopup{};
    bool m_OpenImportProjectPopup{};
    bool m_ProjectNameValidated{ false };

    Ref<Fussion::Texture2D> m_TestTexture{};

    std::vector<EditorProject> m_Projects{};
};
