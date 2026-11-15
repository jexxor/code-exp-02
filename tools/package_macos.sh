#!/bin/bash
set -euo pipefail

bundle=$1
archive=$2
executable="$bundle/Contents/MacOS/white_window"
frameworks="$bundle/Contents/Frameworks"

test -f "$executable"
dylibbundler -od -b -x "$executable" -d "$frameworks" \
  -p '@executable_path/../Frameworks/'

if otool -L "$executable" "$frameworks"/*.dylib |
    grep -E '/(opt/homebrew|usr/local)/(Cellar|opt|lib)/'; then
  echo 'The app still links to Homebrew libraries outside the bundle.' >&2
  exit 1
fi

codesign --force --deep --sign - "$bundle"
mkdir -p "$(dirname "$archive")"
ditto -c -k --sequesterRsrc --keepParent "$bundle" "$archive"
