#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace mumbleshare {

inline constexpr std::string_view kDataId = "ir.havasepehr.mumbleshare/1";
inline constexpr std::size_t kMaximumPayloadBytes = 256;
inline constexpr std::int64_t kInvitationLifetimeSeconds = 120;

struct Invitation {
    std::string transferId;
    std::string room;
    std::int64_t expiresAt = 0;
};

bool isValidRoom(std::string_view room) noexcept;
bool isValidTransferId(std::string_view transferId) noexcept;
std::string serializeInvitation(const Invitation &invitation);
std::optional<Invitation> parseInvitation(
    std::string_view payload,
    std::int64_t now,
    std::string *error = nullptr
) noexcept;

} // namespace mumbleshare
