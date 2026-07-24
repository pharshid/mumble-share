#!/bin/sh
set -eu

: "${MUMBLE_SHARE_HOST:?Set MUMBLE_SHARE_HOST to the certificate hostname}"
lineage="${RENEWED_LINEAGE:-/etc/letsencrypt/live/$MUMBLE_SHARE_HOST}"
destination="/etc/turnserver-certs"

install -d -o root -g turnserver -m 0750 "$destination"
install -o root -g turnserver -m 0640 "$lineage/fullchain.pem" "$destination/fullchain.pem"
install -o root -g turnserver -m 0640 "$lineage/privkey.pem" "$destination/privkey.pem"

systemctl try-restart coturn.service
