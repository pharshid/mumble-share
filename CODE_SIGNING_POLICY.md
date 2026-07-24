# Code signing policy

Mumble Share is applying for the SignPath Foundation open-source program.
Upon acceptance: **Free code signing provided by SignPath.io, certificate by
SignPath Foundation.** Until acceptance, every release is explicitly marked
unsigned and accompanied by SHA-256 checksums and GitHub build provenance.

## Project and roles

- Source repository: <https://github.com/pharshid/mumble-share>
- Committer and reviewer: [Pharshid Imanipour (`@pharshid`)](https://github.com/pharshid)
- Release approver: [Pharshid Imanipour (`@pharshid`)](https://github.com/pharshid)

Repository and signing accounts must use multi-factor authentication. Changes
from non-committers require review by the project reviewer. A release signing
request requires a separate approval through the protected
`release-approval` GitHub environment.

## Build and signing controls

1. Release source is an annotated `v*` tag on the protected `main` branch.
2. GitHub Actions builds and tests the binaries from that tag.
3. Third-party Actions and the Mumble plugin wrapper are pinned to commits.
4. The workflow records SHA-256 checksums and a GitHub provenance attestation.
5. The release approver reviews the candidate before the release job runs.
6. Once enrolled, only artifacts produced by this workflow may be submitted
   to SignPath. Locally built binaries must never be submitted for signing.
7. Signed results must be verified before stable metadata is updated.

Compromise of a maintainer account, build workflow, signing request, or
released artifact must be reported according to
[`SECURITY.md`](SECURITY.md). Affected releases will be withdrawn and SignPath
will be contacted when certificate revocation is appropriate.

## Privacy statement

This program will not transfer any information to other networked systems
unless specifically requested by the user or the person installing or
operating it. Activating a sharing shortcut sends only a temporary room
invitation to users in the current Mumble channel and opens the configured
PairDrop service. File transfer begins only after explicit sender and receiver
actions. See the complete [privacy notice](web/privacy.html).
