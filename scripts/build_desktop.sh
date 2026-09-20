#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 scripts/generate_config.py
mkdir -p build
compiler="${CXX:-c++}"
flags=()
if [[ -z "${CXX:-}" && "$(uname -s)" == Darwin && -x /Library/Developer/CommandLineTools/usr/bin/clang++ ]]; then
  compiler=/Library/Developer/CommandLineTools/usr/bin/clang++
  export DEVELOPER_DIR=/Library/Developer/CommandLineTools
  sdk_path="$(/usr/bin/xcrun --show-sdk-path)"
  flags=(-isysroot "$sdk_path" -isystem "$sdk_path/usr/include/c++/v1")
fi
"$compiler" "${flags[@]}" -std=c++17 -Wall -Wextra -Werror -pedantic -Iinclude simulator/main.cpp -o build/simulator
"$compiler" "${flags[@]}" -std=c++17 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -Iinclude tests/core_test.cpp -o build/core_test
./build/core_test
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 scripts/generate_config.py --check
