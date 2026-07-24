#!/bin/sh
set -eu

lineage="${RENEWED_LINEAGE:-/etc/letsencrypt/live/send.havasepehr.ir}"
destination="/etc/turnserver-certs"

install -d -o root -g turnserver -m 0750 "$destination"
install -o root -g turnserver -m 0640 "$lineage/fullchain.pem" "$destination/fullchain.pem"
install -o root -g turnserver -m 0640 "$lineage/privkey.pem" "$destination/privkey.pem"

systemctl try-restart coturn.service
