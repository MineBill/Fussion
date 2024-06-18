#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorApplication.h"
#include "Layers/ImGuiLayer.h"

#include "Fussion/Renderer/Renderer.h"
#include "Fussion/Input/Input.h"
#include "imgui.h"


#include <glm/gtc/matrix_transform.hpp>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "Fussion/Events/ApplicationEvents.h"
#include "Fussion/Log/FileSink.h"
#include "Fussion/OS/Dialog.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/Scene/Entity.h"
#include "Project/Project.h"

EditorApplication* EditorApplication::s_EditorInstance;

void EditorApplication::OnStart()
{
    ZoneScoped;
    using namespace Fussion;

    s_EditorInstance = this;
    Log::DefaultLogger()->RegisterSink(FileSink::Create("Pepegas.log"));
    LOG_DEBUGF("Current path is: {}", std::filesystem::current_path().string());

    auto path = Dialogs::ShowFilePicker("Project File", {"*.fsnproj"});
    auto loaded = Project::Load(path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    s_EditorInstance = this;
    m_Editor = MakePtr<Editor>();

    m_ImGuiLayer = MakePtr<ImGuiLayer>();
    m_ImGuiLayer->Init();

    m_Editor->OnStart();

    PushLayer(m_ImGuiLayer.get());

    Application::OnStart();
}

void EditorApplication::OnUpdate(const f32 delta)
{
    ZoneScoped;
    using namespace Fussion;

    m_ImGuiLayer->Begin();

    // Update the layers.
    Application::OnUpdate(delta);
    m_Editor->OnUpdate(delta);

    auto [cmd, image] = Renderer::Begin();
    if (cmd == nullptr) {
        m_ImGuiLayer->End(cmd);
        return;
    }

    cmd->Begin();
    auto window_size = Vector2{CAST(f32, m_Window->GetWidth()), CAST(f32, m_Window->GetHeight())};

    m_Editor->OnDraw(cmd);

    const auto main = Renderer::GetInstance()->GetMainRenderPass();
    cmd->BeginRenderPass(main, Renderer::GetInstance()->GetSwapchain()->GetFrameBuffer(image));
    cmd->SetViewport({window_size.x, -window_size.y});
    cmd->SetScissor({0, 0, window_size.x, window_size.y});

    m_ImGuiLayer->End(cmd);

    cmd->EndRenderPass(main);

    cmd->End();

    Renderer::End(cmd);
}

void EditorApplication::OnEvent(Fussion::Event& event)
{
    using namespace Fussion;
    m_Editor->OnEvent(event);
}

void EditorApplication::OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc)
{
    if (m_Editor)
        m_Editor->OnLogReceived(level, message, loc);
}