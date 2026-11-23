# Contributing to Slate

Thanks for helping make Slate better. Bug reports, device reports, packaging
fixes, documentation, and pull requests are all welcome.

## Good first contributions

- Report your distribution, desktop, Qt version, and tablet revision.
- Improve the troubleshooting notes when you hit a problem.
- Add a translation to `src/i18n/en.json` (the loader follows Omarchy's
  language setting).
- Test the hold-to-learn flow with a different keyboard layout and report the
  captured shortcut.
- Package Slate for a distribution that is not listed in
  [docs/platforms.md](docs/platforms.md).

## Development setup

```sh
git clone https://github.com/PhaNtoM-GHosT-11101/slate.git
cd slate
./bin/build
./bin/run
```

Run the test suite:

```sh
./bin/test
```

The tests use Qt Test and do not need a tablet. They cover shortcut parsing,
evdev mapping, Qt key capture normalization, settings block upserts and
rollbacks, JSON persistence, mouse action validation, and theme parsing.

## Project layout

| Path | Purpose |
| --- | --- |
| `src/backend.cpp` | GUI-facing state, Hyprland apply/rollback, service state |
| `src/bindings.cpp` | Express-key model and JSON persistence |
| `src/keymap.cpp` | Combo parser, evdev/wtype/xdotool mappings, Qt capture |
| `src/daemon.cpp` | EVIOCGRAB helper and binding execution |
| `src/mouseinjector.cpp` | Virtual uinput mouse |
| `src/device.cpp` | Event-node discovery |
| `src/pages/` | Mapping, Pen, Express keys, and About pages |
| `src/components/` | Shared QML controls, including `HoldButton` and `CaptureOverlay` |
| `pkgbuild/` | Arch package, service, udev, hwdb, desktop entry, icon |
| `docs/` | Screenshots, platform notes, and GitHub metadata |

## Adding an action

Actions cross several layers. Keep them in sync:

1. Add the action identifier to `SlateBindings::actions()` in
   `src/bindings.cpp`.
2. Validate and normalize it in `load()` and `save()`.
3. Add the UI option in `src/components/KeyRow.qml`.
4. Execute it in `Daemon::trigger()`.
5. Add a test in `tests/test_main.cpp`.

Mouse actions additionally need an entry in `mouseActionList()` and a branch in
`MouseInjector::trigger()`.

## UI work

- Use the existing components (`Card`, `PageHeader`, `AppButton`,
  `NumberField`, `Choice`, `Toggle`) before adding new controls.
- Follow the Omarchy theme via `theme.accent`, `theme.accentForeground`, and
  `theme.radius`; do not hardcode an accent colour.
- Keep the minimum window size usable (1000 × 640). The express-key row is the
  most constrained layout; test it at that size.
- Capture must remain cancellable with `Esc`, and global shortcuts must stay
  disabled while the capture sheet is open.
- Use `ToolTip` and visible labels for gestures that are not obvious. The
  four-second hold is intentionally deliberate; do not replace it with a
  single click.

## Hardware testing

Before sending a driver or input-related change, include:

- Tablet model and USB ID (`lsusb`)
- Desktop and compositor versions (`hyprctl version`, session type)
- Qt version (`qmake6 -query QT_VERSION`)
- The relevant event node names and `getfacl` output
- Whether the helper was running and what `journalctl --user -u slate.service`
  reported

Never include a GitHub token, password, or other secret in logs, screenshots,
or issue text.

## Commits and pull requests

- Keep commits focused and describe the user-visible change.
- Run `./bin/build` and `./bin/test` before opening a pull request.
- Mention the hardware and desktop you tested on.
- For UI changes, attach before/after screenshots. You can generate clean
  screenshots without a compositor:

```sh
tmp="$(mktemp -d)"
mkdir -p "$tmp/config" "$tmp/state"
XDG_CONFIG_HOME="$tmp/config" XDG_STATE_HOME="$tmp/state" \
    QT_QPA_PLATFORM=offscreen ./build/slate --page 2 --screenshot /tmp/keys.png
XDG_CONFIG_HOME="$tmp/config" XDG_STATE_HOME="$tmp/state" \
    QT_QPA_PLATFORM=offscreen ./build/slate --page 2 --capture --screenshot /tmp/learn.png
rm -rf "$tmp"
```

## Packaging

The Arch package lives in `pkgbuild/`. `./bin/package` builds a working-tree
source tarball and runs `makepkg -f`; `./bin/install` also installs the result.
If you add an installed file, update both `pkgbuild/PKGBUILD` and `install.sh`,
then run the dry run:

```sh
./install.sh --dry-run --no-deps
```

## Reporting a bug

Open an issue with:

1. What you expected.
2. What happened.
3. Steps to reproduce.
4. Distribution, desktop, Qt version, and tablet firmware/revision.
5. Relevant logs, with secrets removed.

For a new tablet, include the output of `lsusb`, `hyprctl devices`, the event
node names under `/sys/class/input/`, and its `device/name` files. A device
report is useful even if you cannot write code.
