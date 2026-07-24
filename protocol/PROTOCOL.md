# Invitation protocol version 1

Mumble plugin data identifier:

```text
ir.havasepehr.mumbleshare/1
```

Payload:

```text
v=1;t=invite;id=<32 lowercase hex>;room=<5 lowercase letters>;exp=<unix seconds>
```

Rules:

- Maximum payload length is 256 bytes, well below Mumble's 1 KiB limit.
- Unknown, missing, duplicated, or malformed fields reject the entire message.
- Invitations expire after two minutes.
- Receivers reject timestamps more than five minutes in the future.
- Filenames, paths, file contents, MIME types, and clipboard data are forbidden.
- The helper constructs URLs itself and accepts only five-letter room IDs. The
  plugin cannot instruct it to open an arbitrary URL.
- The receiver must confirm before a browser is opened.
- PairDrop performs a second confirmation before accepting the actual file.

Plugin messages are discovery metadata, not a confidential channel. Mumble's
API documentation states that active plugins on the receiving client can see
plugin data.
