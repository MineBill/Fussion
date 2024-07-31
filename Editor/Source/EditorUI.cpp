#include "EditorUI.h"
#include "Layers/Editor.h"

namespace EUI {
namespace Detail {
ButtonStyle& GetButtonStyle(ButtonStyles style)
{
    return EditorStyle::GetStyle().ButtonStyles[style];
}

WindowStyle& GetWindowStyle(WindowStyles style)
{
    return EditorStyle::GetStyle().WindowStyles[style];
}
}
}
