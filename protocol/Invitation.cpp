#include "protocol/Invitation.h"

#include <charconv>
#include <limits>
#include <map>

namespace mumbleshare {
namespace {

bool isLowerAsciiLetter(char value) noexcept {
    return value >= 'a' && value <= 'z';
}

bool isLowerHex(char value) noexcept {
    return (value >= '0' && value <= '9') || (value >= 'a' && value <= 'f');
}

void setError(std::string *error, const char *message) noexcept {
    if (error) {
        try {
            *error = message;
        } catch (...) {
        }
    }
}

} // namespace

bool isValidRoom(std::string_view room) noexcept {
    if (room.size() != 5) {
        return false;
    }
    for (const char value : room) {
        if (!isLowerAsciiLetter(value)) {
            return false;
        }
    }
    return true;
}

bool isValidTransferId(std::string_view transferId) noexcept {
    if (transferId.size() != 32) {
        return false;
    }
    for (const char value : transferId) {
        if (!isLowerHex(value)) {
            return false;
        }
    }
    return true;
}

std::string serializeInvitation(const Invitation &invitation) {
    if (!isValidTransferId(invitation.transferId) ||
        !isValidRoom(invitation.room) ||
        invitation.expiresAt <= 0) {
        return {};
    }

    return "v=1;t=invite;id=" + invitation.transferId +
           ";room=" + invitation.room +
           ";exp=" + std::to_string(invitation.expiresAt);
}

std::optional<Invitation> parseInvitation(
    std::string_view payload,
    std::int64_t now,
    std::string *error
) noexcept {
    try {
        if (payload.empty() || payload.size() > kMaximumPayloadBytes) {
            setError(error, "payload length is invalid");
            return std::nullopt;
        }

        std::map<std::string, std::string> values;
        std::size_t position = 0;
        while (position < payload.size()) {
            const auto separator = payload.find(';', position);
            const auto part = payload.substr(
                position,
                separator == std::string_view::npos
                    ? payload.size() - position
                    : separator - position
            );
            const auto equals = part.find('=');
            if (equals == std::string_view::npos || equals == 0 || equals + 1 >= part.size()) {
                setError(error, "payload field is malformed");
                return std::nullopt;
            }

            const std::string key(part.substr(0, equals));
            const std::string value(part.substr(equals + 1));
            if (!values.emplace(key, value).second) {
                setError(error, "payload field is duplicated");
                return std::nullopt;
            }

            if (separator == std::string_view::npos) {
                break;
            }
            position = separator + 1;
        }

        if (values.size() != 5 ||
            values["v"] != "1" ||
            values["t"] != "invite") {
            setError(error, "unsupported invitation");
            return std::nullopt;
        }

        Invitation invitation;
        invitation.transferId = values["id"];
        invitation.room = values["room"];
        if (!isValidTransferId(invitation.transferId) || !isValidRoom(invitation.room)) {
            setError(error, "invitation identifiers are invalid");
            return std::nullopt;
        }

        const auto &expiration = values["exp"];
        const char *begin = expiration.data();
        const char *end = expiration.data() + expiration.size();
        const auto conversion = std::from_chars(begin, end, invitation.expiresAt);
        if (conversion.ec != std::errc{} || conversion.ptr != end) {
            setError(error, "expiration is invalid");
            return std::nullopt;
        }

        if (invitation.expiresAt < now - 30 ||
            invitation.expiresAt > now + 5 * 60) {
            setError(error, "invitation has expired or is too far in the future");
            return std::nullopt;
        }

        return invitation;
    } catch (...) {
        setError(error, "invitation parsing failed");
        return std::nullopt;
    }
}

} // namespace mumbleshare
