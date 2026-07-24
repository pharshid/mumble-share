# Operations

## Configuration

Copy `deployment.env.example` to a private deployment file, replace every
example value, and load it before rendering the templates:

```sh
set -a
. /etc/mumble-share/deployment.env
set +a

envsubst '${MUMBLE_SHARE_HOST}' \
  < deploy/nginx-mumble-share.conf.template \
  > /etc/nginx/sites-available/mumble-share
envsubst '${MUMBLE_SHARE_BASE_URL}' \
  < deploy/pairdrop.env.template > /etc/pairdrop/pairdrop.env
envsubst '${MUMBLE_SHARE_HOST} ${TURN_PASSWORD}' \
  < deploy/rtc-config.json.template > /etc/pairdrop/rtc-config.json
envsubst '${MUMBLE_SHARE_HOST} ${MUMBLE_SHARE_PUBLIC_IP}' \
  < deploy/turnserver.conf.template > /etc/turnserver.conf
```

Keep the private deployment file and rendered files out of source control.
`MUMBLE_SHARE_BASE_URL` must end in `/`.

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
curl -fsS "$MUMBLE_SHARE_BASE_URL" >/dev/null
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
2. Remove or restore the deployment-specific DNS record, increment the zone
   serial where applicable, and reload the authoritative DNS service.
3. Restore the previous Mumble server configuration and
   restart `mumble-server` to remove the welcome link.
4. Disable `pairdrop`, `coturn`, and the Nginx site only after the welcome link
   and DNS have been rolled back. Mumble voice itself is independent of these
   services.
