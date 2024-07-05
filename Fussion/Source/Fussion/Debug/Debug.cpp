#include "e5pch.h"
#include "Debug.h"
#include "Fussion/Renderer/Shader.h"

namespace Fussion {
struct DebugData {
    Ref<Shader> DebugShader{};
};

DebugData g_DebugData;

void Debug::Initialize()
{

}

void Debug::DrawLine(Vector3 start, Vector3 end) {}
}
