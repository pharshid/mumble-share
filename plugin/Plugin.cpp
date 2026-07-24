#include "mumble/plugin/MumblePlugin.h"

#include "plugin/ProcessLauncher.h"
#include "protocol/Invitation.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::int64_t unixTimeNow() noexcept {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string randomHex(std::size_t length) {
    static constexpr char alphabet[] = "0123456789abcdef";
    std::random_device random;
    std::string result;
    result.reserve(length);
    for (std::size_t index = 0; index < length; ++index) {
        result.push_back(alphabet[random() % 16]);
    }
    return result;
}

std::string randomRoom() {
    static constexpr char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
    std::random_device random;
    std::string result;
    result.reserve(5);
    for (std::size_t index = 0; index < 5; ++index) {
        result.push_back(alphabet[random() % 26]);
    }
    return result;
}

class MumbleSharePlugin final : public MumblePlugin {
public:
    MumbleSharePlugin()
        : MumblePlugin(
              "Mumble Share",
              "Hava Sepehr",
              "Temporary file-sharing invitations backed by PairDrop."
          ) {}

    mumble_error_t init() noexcept override {
        m_api.log_noexcept(
            "Mumble Share loaded. Grant keyboard monitoring permission to use "
            "Ctrl+Shift+V or Ctrl+Shift+F."
        );
        return MUMBLE_STATUS_OK;
    }

    mumble_version_t getVersion() const noexcept override {
        return {0, 1, 3};
    }

    void onServerDisconnected(mumble_connection_t) noexcept override {
        m_controlDown = false;
        m_shiftDown = false;
        m_vDown = false;
        m_fDown = false;
    }

    bool onReceiveData(
        mumble_connection_t connection,
        mumble_userid_t senderId,
        const uint8_t *data,
        std::size_t dataLength,
        const char *dataId
    ) noexcept override {
        if (!dataId || std::strcmp(dataId, mumbleshare::kDataId.data()) != 0) {
            return false;
        }

        try {
            const std::string_view payload(
                reinterpret_cast<const char *>(data),
                dataLength
            );
            std::string error;
            const auto invitation = mumbleshare::parseInvitation(
                payload, unixTimeNow(), &error
            );
            if (!invitation) {
                const std::string message =
                    "Mumble Share rejected an invitation: " + error;
                m_api.log_noexcept(message.c_str());
                return true;
            }

            const std::string senderName =
                static_cast<std::string>(m_api.getUserName(connection, senderId));
            if (!mumbleshare::launchReceiverHelper(invitation->room, senderName)) {
                const std::string message =
                    "Mumble Share could not start its helper: " +
                    std::string(mumbleshare::lastHelperLaunchError());
                m_api.log_noexcept(message.c_str());
            }
            return true;
        } catch (const std::exception &exception) {
            const std::string message =
                std::string("Mumble Share receive error: ") + exception.what();
            m_api.log_noexcept(message.c_str());
            return true;
        } catch (...) {
            m_api.log_noexcept("Mumble Share receive error.");
            return true;
        }
    }

    void onKeyEvent(uint32_t keyCode, bool wasPress) noexcept override {
        if (keyCode == MUMBLE_KC_CONTROL) {
            m_controlDown = wasPress;
            return;
        }
        if (keyCode == MUMBLE_KC_SHIFT) {
            m_shiftDown = wasPress;
            return;
        }
        if (keyCode == MUMBLE_KC_V) {
            if (wasPress && !m_vDown && m_controlDown && m_shiftDown) {
                beginShare();
            }
            m_vDown = wasPress;
            return;
        }
        if (keyCode == MUMBLE_KC_F) {
            if (wasPress && !m_fDown && m_controlDown && m_shiftDown) {
                beginShare();
            }
            m_fDown = wasPress;
        }
    }

    bool hasUpdate() noexcept override {
        return false;
    }

    MumbleStringWrapper getUpdateDownloadURL() const noexcept override {
        static constexpr char url[] =
            "https://github.com/pharshid/mumble-share/releases/latest";
        return {url, sizeof(url) - 1, false};
    }

    void releaseResource(const void *) noexcept override {
        std::terminate();
    }

private:
    void beginShare() noexcept {
        try {
            const auto connection = m_api.getActiveServerConnection();
            if (!m_api.isConnectionSynchronized(connection)) {
                m_api.log_noexcept("Mumble Share: the server is not ready.");
                return;
            }

            const auto localUser = m_api.getLocalUserID(connection);
            const auto channel = m_api.getChannelOfUser(connection, localUser);
            const auto channelUsers = m_api.getUsersInChannel(connection, channel);

            std::vector<mumble_userid_t> receivers;
            receivers.reserve(channelUsers.size());
            for (const auto user : channelUsers) {
                if (user != localUser) {
                    receivers.push_back(user);
                }
            }

            mumbleshare::Invitation invitation{
                randomHex(32),
                randomRoom(),
                unixTimeNow() + mumbleshare::kInvitationLifetimeSeconds
            };
            const auto payloadText =
                mumbleshare::serializeInvitation(invitation);
            if (payloadText.empty()) {
                m_api.log_noexcept("Mumble Share could not create an invitation.");
                return;
            }

            if (!mumbleshare::launchSenderHelper(invitation.room)) {
                const std::string message =
                    "Mumble Share could not start its helper: " +
                    std::string(mumbleshare::lastHelperLaunchError());
                m_api.log_noexcept(message.c_str());
                return;
            }

            if (!receivers.empty()) {
                const std::vector<uint8_t> payload(
                    payloadText.begin(), payloadText.end()
                );
                m_api.sendData(
                    connection,
                    receivers,
                    payload,
                    mumbleshare::kDataId.data()
                );
            } else {
                m_api.log_noexcept(
                    "Mumble Share opened PairDrop, but nobody else is in this channel."
                );
            }
        } catch (const std::exception &exception) {
            const std::string message =
                std::string("Mumble Share send error: ") + exception.what();
            m_api.log_noexcept(message.c_str());
        } catch (...) {
            m_api.log_noexcept("Mumble Share send error.");
        }
    }

    bool m_controlDown = false;
    bool m_shiftDown = false;
    bool m_vDown = false;
    bool m_fDown = false;
};

} // namespace

MumblePlugin &MumblePlugin::getPlugin() noexcept {
    static MumbleSharePlugin plugin;
    return plugin;
}
