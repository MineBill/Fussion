#include "epch.h"
#include "AssetWindow.h"

#include "imgui.h"

using namespace Fussion;

void AssetWindow::DrawMenuBar()
{
    using namespace ImGui;
    if (BeginMenuBar()) {
        if (BeginMenu("Asset")) {
            if (MenuItem("Save...", "Ctrl+S")) {
                OnSave();
            }
            EndMenu();
        }

        EndMenuBar();
    }
}
