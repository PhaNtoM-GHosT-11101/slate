# GitHub metadata and releases

Slate is easier to find when the repository has a clear description, topics,
release notes, and screenshots. This page records the intended settings.

## Description

> Settings app and express-key helper for the VEIKK A15 drawing tablet on Linux

## Topics

`veikk`, `veikk-a15`, `drawing-tablet`, `tablet-driver`, `hyprland`, `omarchy`,
`qt6`, `qml`, `wayland`, `linux`, `express-keys`, `input-devices`

Apply them with GitHub CLI:

```sh
gh auth login
./bin/setup-github
```

Or edit **About → Topics** on the repository page.

## Release checklist

```sh
./bin/build
./bin/test
git tag -a v0.2.0 -m "Slate 0.2.0"
git push origin main
git push origin v0.2.0
gh release create v0.2.0 \
  --title "Slate 0.2.0" \
  --notes-file docs/release-0.2.0.md
```

Use a source tarball or a distribution package as the release asset. Do not
upload a build that contains local configuration or a desktop user's data.

## Suggested pinned issue

Pin a short "Read me first" issue that links to:

- the install section of the README,
- the platform matrix,
- the troubleshooting section,
- the device report template.

## What not to publish

- GitHub tokens, passwords, or SSH keys
- `~/.config/hypr/input.lua` without reviewing it for personal paths
- journal logs with home-directory paths or usernames you do not want public
- screenshots that reveal private windows, notifications, or documents
