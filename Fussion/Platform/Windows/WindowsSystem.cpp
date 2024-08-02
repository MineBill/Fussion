#include <GLFW/glfw3.h>
#include "Log/Log.h"
#include "OS/System.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Advapi32.lib")

namespace Fussion::System {
    bool PrefersDark()
    {
        return !PrefersLight();
    }

    bool PrefersLight()
    {
        // based on https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application

        // The value is expected to be a REG_DWORD, which is a signed 32-bit little-endian
        auto buffer = std::vector<char>(4);
        auto cb_data = static_cast<DWORD>(buffer.size() * sizeof(char));
        auto res = RegGetValueW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            L"AppsUseLightTheme",
            RRF_RT_REG_DWORD, // expected value type
            nullptr,
            buffer.data(),
            &cb_data);

        if (res != ERROR_SUCCESS) {
            LOG_ERRORF("Error: {}", res);
            return false;
        }

        // convert bytes written to our buffer to an int, assuming little-endian
        auto i = buffer[3] << 24 |
            buffer[2] << 16 |
            buffer[1] << 8 |
            buffer[0];

        return i == 1;
    }
}
