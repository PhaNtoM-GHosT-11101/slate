# Security policy

Slate is a local desktop application. It does not listen on a network port,
send telemetry, or require an account. The security-sensitive parts are the
input helper and the permissions used to read the tablet and inject mouse
events.

## Reporting a vulnerability

Please use GitHub's private security advisory flow:

<https://github.com/PhaNtoM-GHosT-11101/slate/security/advisories/new>

Do not open a public issue for a vulnerability, and do not include tokens,
passwords, SSH keys, or personal paths in reports. If a credential was pasted
into an issue, chat, or log, revoke it at the provider immediately; deleting
the message is not enough.

## What Slate can do

- Read the VEIKK A15 pen and keyboard event nodes when the udev `uaccess` rule
  grants the logged-in user access.
- Grab the tablet's express-key interface while remapping is enabled. The
  normal keyboard is never grabbed.
- Replay keyboard shortcuts through `wtype` (Wayland) or `xdotool` (X11).
- Create a virtual mouse through `/dev/uinput` for mouse-button and scroll
  actions.
- Run commands the user explicitly assigns to an express key, as that user,
  through `/bin/sh -c`.

## Trust boundaries

- **Bindings are code.** A `Command` action runs with your user's privileges.
  Review `~/.config/slate/keys.json` before sharing a home directory or
  restoring someone else's configuration.
- **uinput is input injection.** Access to `/dev/uinput` lets a desktop
  application synthesize pointer and keyboard input for your session. Slate
  uses it only for the mouse actions you assign. Remove the uinput rule or
  leave mouse actions unused if you do not want that capability.
- **udev rules are system-wide.** The installer writes rules under
  `/etc/udev/rules.d`; a package writes them under `/usr/lib/udev/rules.d`.
  They grant the active seat user access to the tablet and, for mouse actions,
  to `/dev/uinput`.
- **The helper has no elevated privileges.** It runs as a systemd user service
  in your session. Do not install it as root or as a system service.

## Supported versions

Security fixes are applied to the latest release on `main`. If you are running
an older source install, rebuild and reinstall before reporting a problem that
may already be fixed.
