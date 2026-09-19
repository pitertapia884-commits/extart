#!/usr/bin/env bash
set -euo pipefail

ROOT="${EXTART_GECKO_ROOT:-$HOME/extart-gecko}"
REPO="https://github.com/mozilla-firefox/firefox.git"

if [[ -e "$ROOT/.git" ]]; then
    echo "Gecko source already exists: $ROOT"
else
    mkdir -p "$(dirname "$ROOT")"
    git clone "$REPO" "$ROOT"
fi

cd "$ROOT"

if [[ ! -x "./mach" ]]; then
    echo "ERROR: Mozilla source tree is incomplete: $ROOT"
    exit 1
fi

cat > mozconfig.extart <<'EOF'
mk_add_options MOZ_OBJDIR=@TOPSRCDIR@/obj-extart

ac_add_options --enable-application=browser
ac_add_options --disable-tests
ac_add_options --disable-debug
ac_add_options --enable-optimize
EOF

echo
echo "Gecko source: $ROOT"
echo "Mozconfig:    $ROOT/mozconfig.extart"
echo
echo "Next:"
echo "  cd $ROOT"
echo "  export MOZCONFIG=$ROOT/mozconfig.extart"
echo "  ./mach bootstrap"
echo "  ./mach build"
echo
echo "This builds Mozilla's source tree for Gecko development."
echo "It does NOT mean EXTART is embedding Firefox yet."
