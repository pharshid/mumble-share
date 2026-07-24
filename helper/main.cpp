#include "protocol/Invitation.h"

#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace {

constexpr const char *kBaseUrl = "https://send.havasepehr.ir/";

std::string roomUrl(const std::string &room) {
    return std::string(kBaseUrl) +
           "?room_id=" + room +
           "&create_if_invalid=1";
}

#ifdef _WIN32

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

std::string wideToUtf8(const std::wstring &input) {
    if (input.empty()) {
        return {};
    }
    const int length = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, input.data(),
        static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr
    );
    if (length <= 0) {
        return {};
    }
    std::string output(static_cast<std::size_t>(length), '\0');
    WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, input.data(),
        static_cast<int>(input.size()), output.data(), length, nullptr, nullptr
    );
    return output;
}

bool openUrl(const std::string &url) {
    const auto wideUrl = utf8ToWide(url);
    const auto result = reinterpret_cast<std::intptr_t>(
        ShellExecuteW(nullptr, L"open", wideUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL)
    );
    return result > 32;
}

int run(const std::string &mode, const std::string &room, const std::string &sender) {
    if (!mumbleshare::isValidRoom(room)) {
        return 2;
    }

    if (mode == "--send-room") {
        if (!openUrl(roomUrl(room))) {
            MessageBoxW(
                nullptr,
                L"Could not open the trusted PairDrop address.",
                L"Mumble Share",
                MB_OK | MB_ICONERROR
            );
            return 3;
        }
        return 0;
    }

    if (mode == "--receive-room") {
        const std::wstring question =
            L"Accept a temporary file-sharing invitation from \"" +
            utf8ToWide(sender.empty() ? "another Mumble user" : sender) +
            L"\"?\n\nYou will still confirm the actual file inside PairDrop.";
        const int answer = MessageBoxW(
            nullptr,
            question.c_str(),
            L"Mumble Share invitation",
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SETFOREGROUND
        );
        if (answer != IDYES) {
            return 0;
        }
        return openUrl(roomUrl(room)) ? 0 : 3;
    }

    return 2;
}

#else

#include <cstdlib>

bool openUrl(const std::string &url) {
    const std::string command = "xdg-open '" + url + "' >/dev/null 2>&1";
    return std::system(command.c_str()) == 0;
}

int run(const std::string &mode, const std::string &room, const std::string &) {
    if (!mumbleshare::isValidRoom(room)) {
        return 2;
    }
    if (mode != "--send-room" && mode != "--receive-room") {
        return 2;
    }
    return openUrl(roomUrl(room)) ? 0 : 3;
}

#endif

} // namespace

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    int argumentCount = 0;
    LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
    if (!arguments) {
        return 2;
    }

    std::string mode;
    std::string room;
    std::string sender;
    if (argumentCount >= 3) {
        mode = wideToUtf8(arguments[1]);
        room = wideToUtf8(arguments[2]);
    }
    for (int index = 3; index + 1 < argumentCount; ++index) {
        if (std::wstring(arguments[index]) == L"--from") {
            sender = wideToUtf8(arguments[index + 1]);
            break;
        }
    }
    LocalFree(arguments);
    return run(mode, room, sender);
}
#else
int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: mumble-share-helper --send-room ROOM\n";
        return 2;
    }
    std::string sender;
    for (int index = 3; index + 1 < argc; ++index) {
        if (std::string(argv[index]) == "--from") {
            sender = argv[index + 1];
            break;
        }
    }
    return run(argv[1], argv[2], sender);
}
#endif
