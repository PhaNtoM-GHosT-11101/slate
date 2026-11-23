# Platform notes

Slate is a Linux application. This page explains what works where, how to
install it on the major distributions, and what to use on Windows and macOS.
For upgrades, per-user installs, and troubleshooting, see
[installation.md](installation.md).

## Support matrix

| Capability | Hyprland | Other Linux Wayland | Linux X11 | Windows | macOS |
| --- | --- | --- | --- | --- | --- |
| Tablet screen mapping | Yes | No | No | No | No |
| Rotation / left-handed | Yes | No | No | No | No |
| Active area / region | Yes | No | No | No | No |
| Pen pressure settings | Yes | No | No | No | No |
| Live pen test | Yes | Yes | Yes | No | No |
| Express-key remapping | Yes | Yes | Yes | OpenTabletDriver | OpenTabletDriver |
| Mouse-button actions | Yes | Yes | Yes | OpenTabletDriver | OpenTabletDriver |

### Why mapping is Hyprland-specific

Slate writes Hyprland's `input:tablet` and `input:tablettool` options into a
managed block in `~/.config/hypr/input.lua` and applies them with
`hyprctl reload`. Other compositors and X11 use different configuration
models, so the Mapping page is intentionally disabled outside Hyprland. The
express-key helper and the live pen test only need evdev and work on any Linux
desktop where the user can read the tablet's event nodes.

## Distribution packages

Slate's installer handles all of these automatically:

```sh
./install.sh
```

The packages below are what it installs, in case you prefer to manage them
yourself. Build packages are only needed when compiling from source.

### Arch Linux

```sh
sudo pacman -S --needed base-devel qt6-base qt6-declarative qt6-wayland wtype xdotool
./bin/build
sudo ./install.sh --no-deps
```

For a proper package, use `./bin/install` or `makepkg -si` in `pkgbuild/`.

### Debian and Ubuntu

```sh
sudo apt update
sudo apt install -y build-essential qt6-base-dev qt6-base-dev-tools \
  qt6-declarative-dev qt6-wayland-dev \
  qml6-module-qtquick qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts qml6-module-qtquick-window \
  qml6-module-qtquick-templates wtype xdotool
./bin/build
sudo ./install.sh --no-deps
```

On releases without a `wtype` package, install
[wtype from source](https://github.com/atx/wtype) or use an X11 session with
`xdotool`.

### Fedora

```sh
sudo dnf install -y gcc-c++ make qt6-qtbase-devel \
  qt6-qtdeclarative-devel qt6-qtwayland-devel wtype xdotool
./bin/build
sudo ./install.sh --no-deps
```

### openSUSE

```sh
sudo zypper install -y gcc-c++ make qt6-base-devel \
  qt6-declarative-devel qt6-declarative-imports qt6-quickcontrols2-devel \
  qt6-wayland-devel wtype xdotool
./bin/build
sudo ./install.sh --no-deps
```

### Other distributions

You need:

- Qt 6 Core, Gui, Qml, Quick, QuickControls2, Wayland, and Network modules
- `qmake6`
- A C++17 compiler and `make`
- `wtype` (Wayland) or `xdotool` (X11)
- udev and, usually, systemd

Then run `./bin/build` and `sudo ./install.sh --no-deps --no-service` if your
init system is not systemd.

## udev, uinput, and permissions

Slate installs two kinds of rules:

- `70-slate-veikk-a15.rules` gives the logged-in desktop user access to the
  A15 pen and keyboard event nodes through `uaccess`.
- `71-slate-uinput.rules` gives the same user access to `/dev/uinput` for the
  virtual mouse used by mouse actions.

If you prefer not to install the uinput rule, Slate still works for keyboard
and command actions; mouse actions will log a clear permission error.

After installation:

```sh
sudo udevadm control --reload-rules
sudo udevadm hwdb --update
sudo udevadm trigger --subsystem-match=input
```

Then reconnect the tablet. The `uaccess` ACL is tied to the logged-in seat;
log out and back in if the ACL is missing.

## Running without systemd

The helper is a normal foreground process:

```sh
slate --daemon
```

Run it from your desktop's autostart mechanism if you do not use systemd. It
finds the Wayland socket under `$XDG_RUNTIME_DIR` itself. Add
`SLATE_REPLAY=wtype` or `SLATE_REPLAY=xdotool` to force a replay backend.

## Windows

Slate does not ship a Windows build. A real Windows driver needs a signed
kernel/filter component, and Slate is deliberately not promising one without a
Windows build and test pipeline.

Use one of these instead:

### OpenTabletDriver (recommended)

1. Download the Windows installer from
   [OpenTabletDriver releases](https://github.com/OpenTabletDriver/OpenTabletDriver/releases).
2. Install and launch OpenTabletDriver.
3. Connect the A15 and select it in the tablet list. The A15 and A15 V2 are
   both listed in the community configurations.
4. Open **Tablet** settings to set the active area, rotation, and mapping.
5. Open **Bindings** to configure pen buttons and **Auxiliary buttons** for the
   express keys. OTD supports keyboard shortcuts, mouse buttons, scroll, and
   plugins.

OpenTabletDriver is open source and cross-platform; the same configuration
concept applies on macOS and Linux. It is a user-mode driver and does not
require a custom kernel component on Windows.

### VEIKK official driver

VEIKK distributes a Windows driver for the A15. Use it if you want the
manufacturer's control panel or firmware utilities. It is not required for
Slate and is not bundled here.

## macOS

OpenTabletDriver is the recommended option. Check the
[tablet support list](https://opentabletdriver.net/Tablets) for the current A15
status and install the macOS package from the releases page. Slate's helper is
Linux-specific.

## Why not a Windows port of Slate?

A useful Windows port would need:

- a signed kernel or filter driver to intercept the tablet's keyboard
  interface,
- a Windows build and test environment,
- an installer and code-signing certificate.

Projects such as [Interception](https://github.com/oblitum/Interception)
provide a signed filter driver and a C API, but integrating and redistributing
it is a separate product decision with licensing and support consequences.
Slate stays honest: Linux gets the native helper, and Windows gets a mature
cross-platform driver rather than an untested kernel component.
