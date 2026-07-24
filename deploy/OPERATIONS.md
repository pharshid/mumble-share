# Operations

## Endpoints

- PairDrop: `https://send.havasepehr.ir/`
- Onboarding: `https://send.havasepehr.ir/mumble/`
- TURN: `turns:send.havasepehr.ir:5349`
- Windows beta: `https://send.havasepehr.ir/mumble/releases/0.1.1/MumbleShare-0.1.1-win-x64.exe`

## Privacy properties

- PairDrop has no application database.
- WebSocket file fallback is disabled.
- TURN relays encrypted WebRTC packets and does not persist files.
- Nginx access logs contain request metadata, not file contents.
- Mumble invitations never include paths, filenames, or file bytes.
- TURN is capped at 625,000 bytes/s per session and 1,500,000 bytes/s
  aggregate to protect Mumble voice traffic.

## Health checks

```sh
systemctl is-active pairdrop nginx coturn mumble-server
curl -fsS http://127.0.0.1:3000/ >/dev/null
curl -fsS https://send.havasepehr.ir/ >/dev/null
ss -lntup | grep -E ':(80|443|3478|5349|64738)\b'
```

Run `turnutils_uclient` with the password in `/etc/pairdrop/turn-password` to
verify an authenticated allocation after TURN configuration or certificate
changes. Do not print that password into logs or tickets.

## Updates

1. Download a reviewed PairDrop release to a staging directory.
2. Run `npm ci --omit=dev` and `npm audit --omit=dev`.
3. Test it on loopback.
4. Replace `/opt/pairdrop` and restart `pairdrop`.
5. Verify Mumble voice and PairDrop health.

Do not use `npm audit fix --force` on the live directory.

## Rollback

1. Restore `/opt/pairdrop` from the previous reviewed directory and restart
   `pairdrop`.
2. Restore `/root/havasepehr.ir.hosts.before-mumble-share-20260724` to
   `/var/lib/bind/havasepehr.ir.hosts`, increment the zone serial, and reload
   BIND if the `send` hostname must be removed.
3. Restore `/etc/mumble/mumble-server.ini.before-mumble-share-20260724` and
   restart `mumble-server` to remove the welcome link.
4. Disable `pairdrop`, `coturn`, and the Nginx site only after the welcome link
   and DNS have been rolled back. Mumble voice itself is independent of these
   services.
