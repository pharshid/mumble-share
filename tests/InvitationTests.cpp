#include "protocol/Invitation.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void require(bool condition, const char *message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main() {
    constexpr std::int64_t now = 1'800'000'000;
    const mumbleshare::Invitation expected{
        "0123456789abcdef0123456789abcdef",
        "abcde",
        now + 120
    };

    const auto encoded = mumbleshare::serializeInvitation(expected);
    require(!encoded.empty(), "valid invitation must serialize");

    std::string error;
    const auto decoded = mumbleshare::parseInvitation(encoded, now, &error);
    require(decoded.has_value(), "serialized invitation must parse");
    require(decoded->transferId == expected.transferId, "transfer ID must round-trip");
    require(decoded->room == expected.room, "room must round-trip");
    require(decoded->expiresAt == expected.expiresAt, "expiration must round-trip");

    require(
        !mumbleshare::parseInvitation(
            "v=1;t=invite;id=0123456789abcdef0123456789abcdef;room=ABCD1;exp=1800000120",
            now
        ),
        "invalid room must be rejected"
    );
    require(
        !mumbleshare::parseInvitation(
            "v=1;t=invite;id=0123456789abcdef0123456789abcdef;room=abcde;exp=1799990000",
            now
        ),
        "expired invitation must be rejected"
    );
    require(
        !mumbleshare::parseInvitation(encoded + ";room=fghij", now),
        "duplicate or extra field must be rejected"
    );
    require(
        !mumbleshare::parseInvitation(std::string(257, 'x'), now),
        "oversized payload must be rejected"
    );

    std::cout << "Invitation protocol tests passed\n";
    return 0;
}
