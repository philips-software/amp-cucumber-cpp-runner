#!/usr/bin/env bash
# Installs clang-cl, lld-link, llvm-lib, xwin and wine. The Windows SDK itself is fetched manually via get-winsdk.sh.
set -euo pipefail

XWIN_VERSION="0.10.0"
XWIN_SHA256="d870eb4b2f390878af6da1ccd3cf321d22fcb72720984853b4be732ae597fc88"
XWIN_ARCHIVE="xwin-${XWIN_VERSION}-x86_64-unknown-linux-musl"

sudo apt-get install -y --no-install-recommends clang lld llvm wine64

LLVM_BIN="$(llvm-config --bindir)"
for tool in clang-cl lld-link llvm-lib; do
    sudo ln -sf "${LLVM_BIN}/${tool}" "/usr/local/bin/${tool}"
done

tmp="$(mktemp -d)"
trap 'rm -rf "${tmp}"' EXIT

curl -fsSL -o "${tmp}/xwin.tar.gz" "https://github.com/Jake-Shadle/xwin/releases/download/${XWIN_VERSION}/${XWIN_ARCHIVE}.tar.gz"
echo "${XWIN_SHA256}  ${tmp}/xwin.tar.gz" | sha256sum --check --strict
tar -xzf "${tmp}/xwin.tar.gz" -C "${tmp}"
sudo install -m 0755 "${tmp}/${XWIN_ARCHIVE}/xwin" /usr/local/bin/xwin
