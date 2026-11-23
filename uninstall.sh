#!/usr/bin/env bash
# Remove a Slate install created by install.sh.
set -euo pipefail

PREFIX="/usr/local"
PURGE=0
DRY_RUN=0
REMOVE_RULES=1

usage() {
    cat <<'EOF'
Usage: ./uninstall.sh [options]

Options:
  --prefix DIR   install prefix used by install.sh (default: /usr/local)
  --purge        also remove ~/.config/slate and ~/.local/state/slate
  --no-udev      leave the system-wide udev/hwdb rules in place
  --dry-run      print what would be removed
  -h, --help     show this help

The Hyprland managed block in ~/.config/hypr/input.lua is never removed
automatically; delete it by hand if you want those settings back.
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix)
            [ $# -ge 2 ] || { echo "uninstall.sh: --prefix needs a directory" >&2; exit 2; }
            PREFIX="${2%/}"
            shift 2
            ;;
        --purge) PURGE=1; shift ;;
        --no-udev) REMOVE_RULES=0; shift ;;
        --dry-run) DRY_RUN=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "uninstall.sh: unknown option: $1" >&2; exit 2 ;;
    esac
done

case "$PREFIX" in
    "$HOME"/*) PREFIX_IS_USER=1 ;;
    *) PREFIX_IS_USER=0 ;;
esac

NEED_ROOT=1
if [ "$PREFIX_IS_USER" -eq 1 ] && [ "$REMOVE_RULES" -eq 0 ]; then
    NEED_ROOT=0
fi

if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
elif [ "$NEED_ROOT" -eq 0 ]; then
    SUDO=""
elif command -v sudo >/dev/null 2>&1; then
    SUDO="sudo"
else
    echo "uninstall.sh: sudo is required to remove /etc/udev rules." >&2
    echo "Use --no-udev for a per-user install that left the rules alone." >&2
    exit 1
fi

run() {
    if [ "$DRY_RUN" -eq 1 ]; then printf '  %s\n' "$*"; else "$@"; fi
}
run_root() {
    if [ "$DRY_RUN" -eq 1 ]; then printf '  %s\n' "$*"; elif [ -n "$SUDO" ]; then $SUDO "$@"; else "$@"; fi
}
run_prefix() {
    if [ "$PREFIX_IS_USER" -eq 1 ]; then
        run "$@"
    else
        run_root "$@"
    fi
}

BIN="$PREFIX/bin/slate"
if command -v pacman >/dev/null 2>&1 && [ -e "$BIN" ] && pacman -Qo "$BIN" >/dev/null 2>&1; then
    echo "uninstall.sh: $BIN is owned by a pacman package." >&2
    echo "Remove it with: sudo pacman -Rns slate" >&2
    exit 1
fi

if command -v systemctl >/dev/null 2>&1; then
    if [ "$(id -u)" -ne 0 ]; then
        echo "==> Disabling the express-key helper"
        run systemctl --user disable --now slate.service 2>/dev/null || true
    else
        echo "    Run 'systemctl --user disable --now slate.service' as your desktop user if needed."
    fi
fi

echo "==> Removing Slate files"
run_prefix rm -f "$BIN"
run_prefix rm -f "$PREFIX/share/applications/slate.desktop"
run_prefix rm -f "$PREFIX/share/icons/hicolor/scalable/apps/slate.svg"
run_prefix rm -f "$PREFIX/share/licenses/slate/LICENSE"
if [ "$PREFIX_IS_USER" -eq 1 ]; then
    run_prefix rm -f "$HOME/.local/share/systemd/user/slate.service"
else
    run_prefix rm -f "$PREFIX/lib/systemd/user/slate.service"
fi
if [ "$REMOVE_RULES" -eq 1 ]; then
    run_root rm -f /etc/udev/rules.d/70-slate-veikk-a15.rules
    run_root rm -f /etc/udev/rules.d/71-slate-uinput.rules
    run_root rm -f /etc/udev/hwdb.d/61-veikk-a15.hwdb
fi
run_prefix rmdir "$PREFIX/share/licenses/slate" 2>/dev/null || true

if [ "$REMOVE_RULES" -eq 1 ] && [ "$DRY_RUN" -eq 0 ] && command -v udevadm >/dev/null 2>&1; then
    run_root udevadm control --reload-rules || true
    run_root udevadm hwdb --update || true
fi

if command -v systemctl >/dev/null 2>&1 && [ "$DRY_RUN" -eq 0 ] && [ "$(id -u)" -ne 0 ]; then
    systemctl --user daemon-reload 2>/dev/null || true
fi

if [ "$PURGE" -eq 1 ]; then
    echo "==> Removing user configuration"
    run rm -rf "$HOME/.config/slate" "$HOME/.local/state/slate"
fi

cat <<EOF

Slate was removed.
$( [ "$PURGE" -eq 1 ] && echo "User configuration was removed." || echo "User bindings were kept in ~/.config/slate." )
The managed tablet block in ~/.config/hypr/input.lua was left untouched.
EOF
