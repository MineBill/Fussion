#include "EditorUI.h"
#include "Layers/Editor.h"

namespace EUI {
namespace Detail {
ButtonStyle& GetButtonStyle(ButtonStyles style)
{
    return Editor::Get().GetStyle().ButtonStyles[style];
}

WindowStyle& GetWindowStyle(WindowStyles style)
{
    return Editor::Get().GetStyle().WindowStyles[style];
}
}
}
