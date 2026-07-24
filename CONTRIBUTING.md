# Contributing

Contributions are welcome through pull requests against `main`.

- Keep file bytes, names, and paths out of Mumble plugin data.
- Preserve explicit receiver consent and the allowlisted HTTPS origin.
- Add or update tests for invitation-protocol changes.
- Do not commit credentials, generated binaries, or build directories.
- Non-committer changes require review by the project reviewer named in
  [`CODE_SIGNING_POLICY.md`](CODE_SIGNING_POLICY.md).

## Local verification

```sh
cmake -S . -B build -G Ninja \
  -DMUMBLE_PLUGIN_CPP_DIR=/path/to/mumble-plugin-cpp \
  -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Report security issues privately according to [`SECURITY.md`](SECURITY.md).
