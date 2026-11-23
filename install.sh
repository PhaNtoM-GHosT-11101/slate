#!/usr/bin/env bash
# Slate source installer for Linux.
#
#   ./install.sh                 install to /usr/local
#   ./install.sh --prefix /usr   overwrite a /usr install
#   ./install.sh --dry-run       show what would happen
#
# The script detects Arch, Debian/Ubuntu, Fedora, and openSUSE, installs the
# build/runtime packages, builds the app, and installs the launcher, helper
# service, udev rule, hwdb entry, and the uinput rule used for mouse actions.
set -euo pipefail

ORIGIN="https://github.com/PhaNtoM-GHosT-11101/slate.git"
PREFIX="/usr/local"
ASSUME_YES=0
DRY_RUN=0
INSTALL_DEPS=1
INSTALL_SERVICE=1
INSTALL_RULES=1

usage() {
    cat <<'EOF'
Usage: ./install.sh [options]

Options:
  --prefix DIR      install under DIR (default: /usr/local)
  --yes             do not prompt
  --no-deps         skip distribution package installation
  --no-service      do not install the systemd user unit
  --no-udev         do not install udev/hwdb rules (no root needed)
  --dry-run         print commands without changing anything
  -h, --help        show this help

A normal install needs root for /usr/local and /etc/udev. A per-user install
with --prefix "$HOME/.local" --no-udev needs no root. Run the script as your
desktop user; it uses sudo only for the system-wide steps.
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix)
            [ $# -ge 2 ] || { echo "install.sh: --prefix needs a directory" >&2; exit 2; }
            PREFIX="${2%/}"
            shift 2
            ;;
        --yes) ASSUME_YES=1; shift ;;
        --no-deps) INSTALL_DEPS=0; shift ;;
        --no-service) INSTALL_SERVICE=0; shift ;;
        --no-udev) INSTALL_RULES=0; shift ;;
        --dry-run) DRY_RUN=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "install.sh: unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

case "$PREFIX" in
    "$HOME"/*) PREFIX_IS_USER=1 ;;
    *) PREFIX_IS_USER=0 ;;
esac

# A per-user prefix does not need root unless we also install system-wide udev
# rules. A system prefix always does.
NEED_ROOT=0
if [ "$PREFIX_IS_USER" -eq 0 ] || [ "$INSTALL_RULES" -eq 1 ]; then
    NEED_ROOT=1
fi

if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
elif [ "$NEED_ROOT" -eq 1 ]; then
    if ! command -v sudo >/dev/null 2>&1; then
        echo "install.sh: sudo is required for this install." >&2
        echo "Run as root, or use ./install.sh --no-udev --prefix \"$HOME/.local\"." >&2
        exit 1
    fi
    SUDO="sudo"
else
    SUDO=""
fi

run() {
    if [ "$DRY_RUN" -eq 1 ]; then
        printf '  %s\n' "$*"
    else
        "$@"
    fi
}

run_root() {
    if [ "$DRY_RUN" -eq 1 ]; then
        printf '  %s\n' "$*"
    elif [ -n "$SUDO" ]; then
        $SUDO "$@"
    else
        "$@"
    fi
}

run_prefix() {
    if [ "$PREFIX_IS_USER" -eq 1 ]; then
        run "$@"
    else
        run_root "$@"
    fi
}

ROOT="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]:-$0}")" && pwd)"
CLEANUP=""
cleanup() {
    if [ -n "$CLEANUP" ] && [ -d "$CLEANUP" ]; then
        rm -rf "$CLEANUP"
    fi
}
trap cleanup EXIT

if [ ! -f "$ROOT/slate.pro" ]; then
    if [ "$DRY_RUN" -eq 1 ]; then
        echo "install.sh: --dry-run must be run from a Slate source tree." >&2
        exit 1
    fi
    echo "==> Downloading Slate"
    if ! command -v git >/dev/null 2>&1; then
        echo "install.sh: git is required to download the source." >&2
        exit 1
    fi
    CLEANUP="$(mktemp -d)"
    run git clone --depth=1 "$ORIGIN" "$CLEANUP/slate"
    ROOT="$CLEANUP/slate"
fi

echo "==> Detecting distribution"
DISTRO="unknown"
if command -v pacman >/dev/null 2>&1; then
    DISTRO="arch"
elif command -v apt-get >/dev/null 2>&1; then
    DISTRO="debian"
elif command -v dnf >/dev/null 2>&1; then
    DISTRO="fedora"
elif command -v zypper >/dev/null 2>&1; then
    DISTRO="suse"
fi
echo "    $DISTRO"

install_deps() {
    case "$DISTRO" in
        arch)
            echo "==> Installing build and runtime packages"
            run_root pacman -S --needed --noconfirm \
                base-devel qt6-base qt6-declarative qt6-wayland wtype xdotool git
            ;;
        debian)
            echo "==> Installing build and runtime packages"
            run_root apt-get update
            run_root apt-get install -y --no-install-recommends \
                build-essential git qt6-base-dev qt6-base-dev-tools \
                qt6-declarative-dev qt6-wayland-dev \
                qml6-module-qtquick qml6-module-qtquick-controls \
                qml6-module-qtquick-layouts qml6-module-qtquick-window \
                qml6-module-qtquick-templates wtype xdotool
            ;;
        fedora)
            echo "==> Installing build and runtime packages"
            run_root dnf install -y \
                gcc-c++ make git qt6-qtbase-devel qt6-qtdeclarative-devel \
                qt6-qtwayland-devel wtype xdotool
            ;;
        suse)
            echo "==> Installing build and runtime packages"
            run_root zypper --non-interactive install \
                gcc-c++ make git qt6-base-devel qt6-declarative-devel \
                qt6-declarative-imports qt6-quickcontrols2-devel \
                qt6-wayland-devel wtype xdotool
            ;;
        *)
            echo "install.sh: unknown package manager; install Qt 6 development"
            echo "packages, qmake6, a C++17 compiler, make, and wtype yourself." >&2
            ;;
    esac
}

if [ "$INSTALL_DEPS" -eq 1 ]; then
    install_deps
fi

missing=""
for tool in qmake6 make g++; do
    command -v "$tool" >/dev/null 2>&1 || missing="$missing $tool"
done
if [ -n "$missing" ]; then
    echo "install.sh: missing build tools:$missing" >&2
    echo "Install the Qt 6 development packages and a C++ compiler, then retry." >&2
    exit 1
fi

echo "==> Building Slate"
if [ "$DRY_RUN" -eq 1 ]; then
    echo "  $ROOT/bin/build"
else
    "$ROOT/bin/build"
fi

BIN="$ROOT/build/slate"
for tool in wtype xdotool; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "    note: $tool is not installed; some key replay modes will be unavailable."
    fi
done

echo "==> Installing files under $PREFIX"
run_prefix install -Dm755 "$BIN" "$PREFIX/bin/slate"
run_prefix install -Dm644 "$ROOT/pkgbuild/slate.svg" \
    "$PREFIX/share/icons/hicolor/scalable/apps/slate.svg"
run_prefix install -Dm644 "$ROOT/LICENSE" "$PREFIX/share/licenses/slate/LICENSE"

DESKTOP="$(mktemp)"
sed "s|^Exec=slate$|Exec=$PREFIX/bin/slate|" "$ROOT/pkgbuild/slate.desktop" > "$DESKTOP"
run_prefix install -Dm644 "$DESKTOP" "$PREFIX/share/applications/slate.desktop"
rm -f "$DESKTOP"

if [ "$INSTALL_SERVICE" -eq 1 ]; then
    if [ "$PREFIX_IS_USER" -eq 1 ]; then
        UNIT_DIR="$HOME/.local/share/systemd/user"
    else
        UNIT_DIR="$PREFIX/lib/systemd/user"
    fi
    UNIT="$(mktemp)"
    sed "s|^ExecStart=/usr/bin/slate|ExecStart=$PREFIX/bin/slate|" \
        "$ROOT/pkgbuild/slate.service" > "$UNIT"
    run_prefix install -Dm644 "$UNIT" "$UNIT_DIR/slate.service"
    rm -f "$UNIT"
    echo "    installed $UNIT_DIR/slate.service"
fi

if [ "$INSTALL_RULES" -eq 1 ]; then
    echo "==> Installing device rules"
    run_root install -Dm644 "$ROOT/pkgbuild/70-slate-veikk-a15.rules" \
        /etc/udev/rules.d/70-slate-veikk-a15.rules
    run_root install -Dm644 "$ROOT/pkgbuild/71-slate-uinput.rules" \
        /etc/udev/rules.d/71-slate-uinput.rules
    run_root install -Dm644 "$ROOT/pkgbuild/61-veikk-a15.hwdb" \
        /etc/udev/hwdb.d/61-veikk-a15.hwdb
    if [ "$DRY_RUN" -eq 1 ]; then
        echo "  udevadm control --reload-rules"
        echo "  udevadm hwdb --update"
        echo "  udevadm trigger --subsystem-match=input"
    else
        run_root udevadm control --reload-rules || true
        run_root udevadm hwdb --update || true
        run_root udevadm trigger --subsystem-match=input || true
    fi
fi

if [ "$INSTALL_SERVICE" -eq 1 ] && command -v systemctl >/dev/null 2>&1; then
    if [ "$DRY_RUN" -eq 0 ]; then
        systemctl --user daemon-reload 2>/dev/null || true
    fi
    ENABLE=0
    if [ "$ASSUME_YES" -eq 1 ]; then
        ENABLE=1
    elif [ -t 0 ]; then
        printf 'Enable the express-key helper now? [y/N] '
        read -r answer || true
        case "$answer" in y|Y|yes|YES) ENABLE=1 ;; esac
    fi
    if [ "$ENABLE" -eq 1 ] && [ "$(id -u)" -ne 0 ]; then
        echo "==> Enabling the express-key helper"
        run systemctl --user enable --now slate.service
    else
        echo "    Enable remapping in the app, or run:"
        echo "      systemctl --user enable --now slate.service"
    fi
fi

cat <<EOF

Slate is installed.

  1. Start it from the app launcher or run: $PREFIX/bin/slate
  2. Reconnect the tablet so udev applies the new access rules.
  3. Turn on "Remap express keys" on the Express keys page.

Mouse actions need the uinput rule installed by this script. Shortcuts on
Wayland need wtype; on X11 they need xdotool. See docs/platforms.md for
Windows, macOS, and other-desktop guidance, or docs/installation.md for
per-user installs and upgrades.
EOF
