# Packaging

The AUR package is published as **`slate-veikk`** because `slate`, `slate-bin`
and `slate-git` are already taken in the AUR by an unrelated Qt pixel-art
editor.

`PKGBUILD`, `.SRCINFO` and `slate-veikk.install` in this directory are the
canonical copy. Verify them locally before every release:

```sh
makepkg -f          # builds and packages
makepkg --printsrcinfo > .SRCINFO
```

## Publishing a release

1. Bump `pkgver` and reset `pkgrel=1` in `PKGBUILD`.
2. Update `sha256sums` from the new tag's tarball:

   ```sh
   curl -sSL -o /tmp/slate.tar.gz \
     https://github.com/PhaNtoM-GHosT-11101/slate/archive/v$pkgver.tar.gz
   sha256sum /tmp/slate.tar.gz
   ```

3. Regenerate metadata: `makepkg --printsrcinfo > .SRCINFO`
4. Push to the AUR:

   ```sh
   git clone ssh://aur@aur.archlinux.org/slate-veikk.git
   cp PKGBUILD .SRCINFO slate-veikk.install slate-veikk/
   cd slate-veikk
   makepkg --printsrcinfo > .SRCINFO
   git add -A && git commit -m "upgpkg: slate-veikk $pkgver-1" && git push
   ```

The AUR requires an account at <https://aur.archlinux.org> with an SSH key
added under *My Account*.
