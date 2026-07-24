#include "config/ServiceUrl.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void setServiceUrl(const char *value) {
#ifdef _WIN32
    _putenv_s("MUMBLE_SHARE_BASE_URL", value ? value : "");
#else
    if (value) {
        setenv("MUMBLE_SHARE_BASE_URL", value, 1);
    } else {
        unsetenv("MUMBLE_SHARE_BASE_URL");
    }
#endif
}

bool expectEqual(
    const std::string &actual,
    const std::string &expected,
    const char *testName
) {
    if (actual == expected) {
        return true;
    }
    std::cerr << testName << ": expected " << expected
              << ", got " << actual << '\n';
    return false;
}

} // namespace

int main() {
    bool passed = true;

    setServiceUrl(nullptr);
    passed &= expectEqual(
        mumbleshare::serviceBaseUrl(),
        "https://pairdrop.net/",
        "default URL"
    );

    setServiceUrl("https://share.example.com/pairdrop///");
    passed &= expectEqual(
        mumbleshare::serviceBaseUrl(),
        "https://share.example.com/pairdrop/",
        "configured URL normalization"
    );
    passed &= expectEqual(
        mumbleshare::roomUrl("abcde"),
        "https://share.example.com/pairdrop/?room_id=abcde&create_if_invalid=1",
        "room URL"
    );

    setServiceUrl("javascript:alert(1)");
    passed &= expectEqual(
        mumbleshare::serviceBaseUrl(),
        "https://pairdrop.net/",
        "invalid scheme"
    );

    setServiceUrl("https://share.example.com/?redirect=elsewhere");
    passed &= expectEqual(
        mumbleshare::serviceBaseUrl(),
        "https://pairdrop.net/",
        "query string rejection"
    );

    setServiceUrl(nullptr);
    return passed ? 0 : 1;
}
