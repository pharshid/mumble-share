# Changelog

## 0.1.2 - 2026-07-24

- Add Windows version resources to the plugin and helper.
- Add code-signing governance, third-party notices, and contributor guidance.
- Pin GitHub Actions by commit and add a manually approved release job.
- Publish build-provenance attestations for tagged installers.

## 0.1.1 - 2026-07-24

- Promote the Windows x64 package to the private server's stable channel while
  retaining an explicit unsigned-release warning.
- Find the companion helper through both the loaded plugin path and the
  installer's recorded application directory.
- Add AppData fallback locations for portable and copied Mumble installations.
- Fall back to the plugin's built-in trusted-browser launcher and receiver
  confirmation if the separate helper is unavailable.
- Report the actual helper discovery or Windows process-launch failure in
  Mumble's log.

## 0.1.0 - 2026-07-24

- Add short-lived, channel-scoped PairDrop invitations.
- Add Windows x64 helper and per-user NSIS installer.
- Keep file names, paths, and bytes out of Mumble plugin data.
- Require receiver approval before opening PairDrop.
- Add strict invitation parsing and expiry tests.
