# Installing Slate

Slate is a Qt 6 application with a small systemd user helper. The source
installer is the recommended path on any Linux distribution; distribution
packages and per-user installs are also supported.

## Quick install

```sh
curl -fsSL https://raw.githubusercontent.com/PhaNtoM-GHosT-11101/slate/main/install.sh | bash
```

The script:

1. detects Arch, Debian/Ubuntu, Fedora, or openSUSE and installs the build and
   runtime packages it needs,
2. builds Slate,
3. installs the app, icon, launcher entry, and systemd user unit under
   `/usr/local`,
4. installs the udev rule, hwdb entry, and `/dev/uinput` rule,
5. asks whether to enable the express-key helper.

It must be able to use `sudo` for `/usr/local` and `/etc/udev`. Reconnect the
tablet after it finishes so udev applies the new rules.

### From a clone

```sh
git clone https://github.com/PhaNtoM-GHosT-11101/slate.git
cd slate
./install.sh
```

### Per-user install

For a home-directory install that needs no root and does not touch udev:

```sh
./install.sh --prefix "$HOME/.local" --no-udev
```

This installs the launcher, icon, and systemd unit under
`~/.local/share/systemd/user`. The helper will start only when you enable
remapping in the app. The existing system udev rule for the tablet and
`/dev/uinput` must already be present for full functionality; see
[platforms.md](platforms.md).

### Dry run

```sh
./install.sh --dry-run --no-deps
```

## Arch Linux package

```sh
git clone https://github.com/PhaNtoM-GHosT-11101/slate.git
cd slate
./bin/install          # builds a source tarball and runs makepkg -fsi
```

Or, with `pkgbuild/` copied into a package repository:

```sh
cd pkgbuild
makepkg -fsi
```

The package installs under `/usr` and includes the systemd user unit, udev
rules, hwdb entry, desktop entry, and icon.

## Dependencies

| Distribution | Build packages | Runtime notes |
| --- | --- | --- |
| Arch | `base-devel qt6-base qt6-declarative qt6-wayland` | `wtype` on Wayland, `xdotool` on X11 |
| Debian/Ubuntu | `build-essential qt6-base-dev qt6-base-dev-tools qt6-declarative-dev qt6-wayland-dev` plus the `qml6-module-qtquick-*` packages | `wtype` may need a newer release or a source build |
| Fedora | `gcc-c++ make qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtwayland-devel` | `wtype` and `xdotool` from the repositories |
| openSUSE | `gcc-c++ make qt6-base-devel qt6-declarative-devel qt6-declarative-imports qt6-quickcontrols2-devel qt6-wayland-devel` | `wtype` and `xdotool` where packaged |

The installer handles these lists automatically. If you use another
distribution, install Qt 6 Core, Gui, Qml, Quick, QuickControls2, Wayland, and
Network modules, `qmake6`, a C++17 compiler, `make`, and `wtype` or `xdotool`.

## After installing

```sh
slate                       # or launch Slate from the app launcher
systemctl --user status slate.service
```

1. Reconnect the tablet.
2. Open **Express keys**.
3. Turn on **Remap express keys**.
4. Hold a row's circular **4s** button, then press a key or mouse button.

## Upgrading a source install

```sh
git pull
./install.sh
systemctl --user daemon-reload
sudo udevadm control --reload-rules
```

Your `~/.config/slate/keys.json` and the managed block in
`~/.config/hypr/input.lua` are preserved.

## Uninstalling

```sh
./uninstall.sh
```

For a per-user install:

```sh
./uninstall.sh --prefix "$HOME/.local"
```

Add `--purge` to remove `~/.config/slate` and `~/.local/state/slate` as well.
Add `--no-udev` when the system-wide rules should stay installed, for example
when another copy of Slate or a distribution package still uses them. The
managed Hyprland block is never removed automatically.

## Troubleshooting installs

**`qmake6: command not found`** — install the Qt 6 development package for
your distribution. On Debian/Ubuntu it is provided by `qt6-base-dev-tools`.

**The helper toggle is disabled** — the app could not find
`slate.service`. Re-run the installer, or check:

```sh
systemctl --user list-unit-files 'slate*'
ls /usr/lib/systemd/user/slate.service /usr/local/lib/systemd/user/slate.service
```

**The tablet shows a permission error** — reconnect it, then run:

```sh
sudo udevadm control --reload-rules
sudo udevadm trigger --subsystem-match=input
getfacl /dev/input/event* | grep -A2 -B2 veikk
```

If the ACL is still missing, log out and back in so the `uaccess` seat ACL is
reapplied.
