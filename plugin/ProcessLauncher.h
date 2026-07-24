#pragma once

#include <string>
#include <string_view>

namespace mumbleshare {

bool launchSenderHelper(std::string_view room) noexcept;
bool launchReceiverHelper(std::string_view room, const std::string &senderName) noexcept;
std::string_view lastHelperLaunchError() noexcept;

} // namespace mumbleshare
