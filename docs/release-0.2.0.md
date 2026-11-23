# Slate 0.2.0

## Highlights

- Hold-to-learn express keys: hold the circular **4s** button, then press any
  keyboard shortcut or mouse button.
- Mouse actions: left, right, middle, back, forward, and four-way scroll via a
  virtual uinput mouse.
- Redesigned interface with a persistent sidebar, page hierarchy, status cards,
  a dedicated About page, and a capture sheet with live modifier feedback.
- Cross-distribution source installer for Arch, Debian/Ubuntu, Fedora, and
  openSUSE, with per-user install and dry-run modes.
- Broader A15 event-node matching for firmware and naming variants.
- X11 replay through `xdotool`, with automatic Wayland/X11 selection and a
  `SLATE_REPLAY` override.
- New installation guide, platform notes, security policy, contribution guide,
  issue templates, and fresh screenshots.

## Upgrade notes

- The package now installs a udev rule for `/dev/uinput` so mouse actions work
  without adding the user to the `input` group. Reconnect the tablet after
  upgrading.
- Existing `~/.config/slate/keys.json` files remain compatible. Keyboard and
  command actions keep working; mouse actions are new.
- `slate.service` still lives in the systemd user unit directory. Run
  `systemctl --user daemon-reload` after upgrading a source install.
- The source installer now supports `--prefix "$HOME/.local" --no-udev` for a
  rootless per-user install. See [installation.md](installation.md) for the
  upgrade and uninstall paths.
- Mouse actions need the new `/dev/uinput` rule. Users who installed Slate
  before this release should reinstall the rules or copy
  `pkgbuild/71-slate-uinput.rules` into `/etc/udev/rules.d/`.

## Known limitations

- Screen mapping requires Hyprland's Lua configuration layout.
- Windows and macOS are served by OpenTabletDriver or the official VEIKK
  driver; Slate does not ship a Windows kernel driver.
- Mouse actions require access to `/dev/uinput`.
