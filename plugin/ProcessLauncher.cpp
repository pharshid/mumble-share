#include "plugin/ProcessLauncher.h"
#include "config/ServiceUrl.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace mumbleshare {
namespace {

thread_local std::string g_lastLaunchError;

void setLastLaunchError(const std::string &message) noexcept {
    try {
        g_lastLaunchError = message;
    } catch (...) {
        g_lastLaunchError.clear();
    }
}

std::wstring utf8ToWide(const std::string &input) {
    if (input.empty()) {
        return {};
    }
    const int length = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
        static_cast<int>(input.size()), nullptr, 0
    );
    if (length <= 0) {
        return {};
    }
    std::wstring output(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
        static_cast<int>(input.size()), output.data(), length
    );
    return output;
}

std::wstring quoteArgument(const std::wstring &input) {
    std::wstring output = L"\"";
    std::size_t slashes = 0;
    for (const wchar_t value : input) {
        if (value == L'\\') {
            ++slashes;
            continue;
        }
        if (value == L'"') {
            output.append(slashes * 2 + 1, L'\\');
            output.push_back(L'"');
            slashes = 0;
            continue;
        }
        output.append(slashes, L'\\');
        slashes = 0;
        output.push_back(value);
    }
    output.append(slashes * 2, L'\\');
    output.push_back(L'"');
    return output;
}

std::filesystem::path pluginDirectory() {
    HMODULE module = nullptr;
    const auto address = reinterpret_cast<LPCWSTR>(&pluginDirectory);
    if (!GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            address,
            &module)) {
        return {};
    }

    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(
        module, buffer.data(), static_cast<DWORD>(buffer.size())
    );
    if (length == 0 || length >= buffer.size()) {
        return {};
    }
    return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
}

std::filesystem::path installedDirectory() {
    std::vector<wchar_t> buffer(32768);
    DWORD byteLength = static_cast<DWORD>(buffer.size() * sizeof(wchar_t));
    const LSTATUS status = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\MumbleShare",
        L"InstallDir",
        RRF_RT_REG_SZ,
        nullptr,
        buffer.data(),
        &byteLength
    );
    if (status != ERROR_SUCCESS || byteLength <= sizeof(wchar_t)) {
        return {};
    }
    return std::filesystem::path(buffer.data());
}

std::filesystem::path environmentDirectory(const wchar_t *variable) {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetEnvironmentVariableW(
        variable, buffer.data(), static_cast<DWORD>(buffer.size())
    );
    if (length == 0 || length >= buffer.size()) {
        return {};
    }
    return std::filesystem::path(std::wstring(buffer.data(), length));
}

std::filesystem::path findHelper() {
    const std::vector<std::filesystem::path> directories{
        pluginDirectory(),
        installedDirectory(),
        environmentDirectory(L"LOCALAPPDATA") / L"MumbleShare",
        environmentDirectory(L"APPDATA") / L"Mumble" / L"Plugins"
    };

    std::error_code error;
    for (const auto &directory : directories) {
        if (directory.empty()) {
            continue;
        }
        const auto candidate = directory / L"mumble-share-helper.exe";
        if (std::filesystem::is_regular_file(candidate, error)) {
            return candidate;
        }
        error.clear();
    }
    return {};
}

bool launch(const std::vector<std::wstring> &arguments) noexcept {
    try {
        g_lastLaunchError.clear();
        const auto executable = findHelper();
        if (executable.empty()) {
            setLastLaunchError(
                "helper executable is absent from the plugin and installation directories"
            );
            return false;
        }

        std::wstring commandLine = quoteArgument(executable.wstring());
        for (const auto &argument : arguments) {
            commandLine.push_back(L' ');
            commandLine += quoteArgument(argument);
        }

        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION process{};
        const BOOL created = CreateProcessW(
            executable.c_str(),
            commandLine.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            executable.parent_path().c_str(),
            &startup,
            &process
        );
        if (!created) {
            setLastLaunchError(
                "CreateProcessW failed with Windows error " +
                std::to_string(GetLastError())
            );
            return false;
        }

        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return true;
    } catch (const std::exception &exception) {
        setLastLaunchError(
            std::string("helper discovery failed: ") + exception.what()
        );
        return false;
    } catch (...) {
        setLastLaunchError("helper discovery failed unexpectedly");
        return false;
    }
}

std::wstring roomUrl(std::string_view room) {
    return utf8ToWide(mumbleshare::roomUrl(room));
}

bool openRoomDirectly(std::string_view room) noexcept {
    try {
        const auto url = roomUrl(room);
        const auto result = reinterpret_cast<std::intptr_t>(
            ShellExecuteW(
                nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL
            )
        );
        if (result <= 32) {
            setLastLaunchError(
                "the helper could not start and the browser fallback failed "
                "with ShellExecute code " + std::to_string(result)
            );
            return false;
        }
        g_lastLaunchError.clear();
        return true;
    } catch (...) {
        setLastLaunchError(
            "the helper could not start and the browser fallback failed"
        );
        return false;
    }
}

} // namespace

bool launchSenderHelper(std::string_view room) noexcept {
    if (launch({L"--send-room", utf8ToWide(std::string(room))})) {
        return true;
    }
    return openRoomDirectly(room);
}

bool launchReceiverHelper(std::string_view room, const std::string &senderName) noexcept {
    if (launch({
        L"--receive-room",
        utf8ToWide(std::string(room)),
        L"--from",
        utf8ToWide(senderName)
    })) {
        return true;
    }

    try {
        const std::wstring question =
            L"Accept a temporary file-sharing invitation from \"" +
            utf8ToWide(
                senderName.empty() ? "another Mumble user" : senderName
            ) +
            L"\"?\n\nYou will still confirm the actual file inside PairDrop.";
        const int answer = MessageBoxW(
            nullptr,
            question.c_str(),
            L"Mumble Share invitation",
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SETFOREGROUND
        );
        if (answer != IDYES) {
            g_lastLaunchError.clear();
            return true;
        }
        return openRoomDirectly(room);
    } catch (...) {
        setLastLaunchError(
            "the helper could not start and the invitation fallback failed"
        );
        return false;
    }
}

std::string_view lastHelperLaunchError() noexcept {
    return g_lastLaunchError;
}

} // namespace mumbleshare

#else

namespace mumbleshare {

bool launchSenderHelper(std::string_view) noexcept {
    return false;
}

bool launchReceiverHelper(std::string_view, const std::string &) noexcept {
    return false;
}

std::string_view lastHelperLaunchError() noexcept {
    return "desktop helper is only available in the Windows beta";
}

} // namespace mumbleshare

#endif
