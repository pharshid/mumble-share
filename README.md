# Mumble Share

[![Windows build](https://github.com/pharshid/mumble-share/actions/workflows/windows-release.yml/badge.svg)](https://github.com/pharshid/mumble-share/actions/workflows/windows-release.yml)

Mumble Share adds small, versioned file-transfer invitations to Mumble while
PairDrop carries the actual files over WebRTC.

The project deliberately keeps file data out of Mumble:

- Mumble plugin data contains only a temporary room invitation.
- PairDrop performs the peer-to-peer transfer.
- TURN is an encrypted relay when a direct WebRTC connection cannot be made.
- PairDrop WebSocket file fallback is disabled.

## Repository layout

- `plugin/`: Mumble client plugin.
- `helper/`: small desktop companion for browser launch and notifications.
- `protocol/`: invitation protocol and security rules.
- `packaging/`: Windows installer and CI packaging.
- `web/`: onboarding/download page.
- `deploy/`: systemd, Nginx, PairDrop, and TURN deployment files.
- [`CODE_SIGNING_POLICY.md`](CODE_SIGNING_POLICY.md): signing governance and
  release provenance requirements.
- [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md): dependency attribution.

## Current target

The first supported desktop target is Windows x64 with Mumble 1.5.x. The
invitation protocol is platform-independent so Linux and macOS helpers can be
added without changing the server.

## User workflow

No plugin is required: open a PairDrop instance, create a temporary public
room, and share its link in Mumble.

The optional Windows integration reduces the room-link steps:

1. Install the package, restart Mumble, then check both **Enable** and
   **KeyEvents** for **Mumble Share** in Mumble's plugin settings.
2. In a channel, press **Ctrl+Shift+V** or **Ctrl+Shift+F**. The plugin opens
   a shared PairDrop room for you and sends a short-lived invitation to the
   other users in that channel.
3. Paste or choose the file in the browser. Receivers approve the invitation,
   then approve the actual transfer in PairDrop.

Ordinary **Ctrl+V** image messages stay native to Mumble. Files, videos, and
large images—including 10 MB images—use PairDrop. Browsers do not allow a
plugin to silently read and upload clipboard files, so the sender still pastes
or selects the file once in PairDrop.

## PairDrop service configuration

The plugin uses `https://pairdrop.net/` by default. Self-hosters can select
their own PairDrop-compatible service with the `MUMBLE_SHARE_BASE_URL`
environment variable:

```powershell
[Environment]::SetEnvironmentVariable(
  "MUMBLE_SHARE_BASE_URL",
  "https://share.example.com/",
  "User"
)
```

Restart Mumble after changing the variable. Every participant should use the
same service URL. The value must be an HTTP(S) base URL without a query string
or fragment; invalid values fall back to the public PairDrop instance.

## Build

The plugin uses the official `mumble-plugin-cpp` wrapper:

```sh
cmake -S . -B build -G Ninja \
  -DMUMBLE_PLUGIN_CPP_DIR=/path/to/mumble-plugin-cpp
cmake --build build
ctest --test-dir build --output-on-failure
```

## Service deployment

Deployment files are templates. Secrets are generated on the server and are
not stored in this repository. See `deploy/OPERATIONS.md`.

## Release status

Version 0.1.3 is the stable Windows x64 release. It makes the PairDrop service
configurable for general use. Releases are built through the public CI
pipeline with GitHub build-provenance attestation and remain
unsigned, so users must verify the published SHA-256 digest and may see a
Windows SmartScreen warning until SignPath Foundation enrollment is approved.

## Code signing policy

The project’s team roles, privacy statement, release approval process, and
SignPath attribution are documented in the
[code signing policy](CODE_SIGNING_POLICY.md).
