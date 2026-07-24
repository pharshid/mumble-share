#pragma once

#include <string>
#include <string_view>

namespace mumbleshare {

inline constexpr std::string_view kDefaultServiceBaseUrl =
    "https://pairdrop.net/";

// Reads MUMBLE_SHARE_BASE_URL on every call. Invalid values fall back to the
// public PairDrop instance. The URL must use HTTP(S), contain an authority,
// and must not contain a query string, fragment, whitespace, or backslash.
std::string serviceBaseUrl();

std::string roomUrl(std::string_view room);

} // namespace mumbleshare
