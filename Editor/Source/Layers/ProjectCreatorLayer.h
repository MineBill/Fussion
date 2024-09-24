#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Layer.h"

struct EditorProject {
    std::string name{};
    std::filesystem::path location{};
};

class ProjectCreatorLayer final : public Fussion::Layer {
public:
    virtual void on_start() override;
    virtual void on_update(f32 delta) override;
    virtual void on_event(Fussion::Event& event) override;

    void serialize(Fussion::Serializer& s) const;
    void deserialize(Fussion::Deserializer& ds);

    void add_project(std::filesystem::path const& path);
    void save_projects() const;
    void load_projects();

private:
    bool m_open_new_project_popup{};
    bool m_open_import_project_popup{};
    bool m_project_name_validated{ false };

    std::vector<EditorProject> m_projects{};
};
