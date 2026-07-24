#include "config/ServiceUrl.h"

#include <cstdlib>

namespace mumbleshare {
namespace {

bool hasHttpScheme(std::string_view value) {
    return value.rfind("https://", 0) == 0 ||
           value.rfind("http://", 0) == 0;
}

bool isValidBaseUrl(std::string_view value) {
    if (!hasHttpScheme(value)) {
        return false;
    }

    const auto authorityStart = value.find("://") + 3;
    if (authorityStart >= value.size() || value[authorityStart] == '/') {
        return false;
    }

    for (const unsigned char character : value) {
        if (character <= 0x20 || character == 0x7f ||
            character == '"' || character == '\'' ||
            character == '`' || character == '\\' ||
            character == '?' || character == '#') {
            return false;
        }
    }
    return true;
}

std::string normalizeBaseUrl(std::string value) {
    while (!value.empty() && value.back() == '/') {
        value.pop_back();
    }
    value.push_back('/');
    return value;
}

} // namespace

std::string serviceBaseUrl() {
    const char *configured = std::getenv("MUMBLE_SHARE_BASE_URL");
    if (configured && isValidBaseUrl(configured)) {
        return normalizeBaseUrl(configured);
    }
    return std::string(kDefaultServiceBaseUrl);
}

std::string roomUrl(std::string_view room) {
    return serviceBaseUrl() +
           "?room_id=" + std::string(room) +
           "&create_if_invalid=1";
}

} // namespace mumbleshare
