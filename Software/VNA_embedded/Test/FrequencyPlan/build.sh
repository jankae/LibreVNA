#!/bin/bash
# Build fwplan against one firmware tree.
#   build.sh FIRMWARE_REPO OUT_BINARY [extra g++ flags, e.g. -DPATCH_LO2_EXACT]
# The driver sources are copied next to the stub headers, because the drivers include
# "stm.hpp" with quotes and would otherwise pick up the real HAL header beside them.
# Read from the tree itself, unless given as flags:
#   FW_V164      set when Si5351C::SetPLL has no exactFrequency argument (1.6.4 and older)
#   LO2_TOL_DIV  the n in VNA.cpp's "IFdeviation > IFBW / n"
set -e
here=$(cd "$(dirname "$0")" && pwd)
repo=$1; out=$2; shift 2
app=$repo/Software/VNA_embedded/Application
drv=$app/Drivers
b=$(mktemp -d)
trap 'rm -rf "$b"' EXIT
cp "$drv"/Si5351C.cpp "$drv"/Si5351C.hpp "$drv"/max2871.cpp "$drv"/max2871.hpp \
   "$drv"/algorithm.cpp "$drv"/algorithm.hpp "$b"/
cp "$here"/stubs/*.h* "$b"/
flags=()
grep -q "exactFrequency" "$drv"/Si5351C.hpp || flags+=(-DFW_V164)
case " $* " in *" -DLO2_TOL_DIV="*) ;; *)
  tol=$(sed -n 's/.*IFdeviation > IFBW \/ \([0-9]*\).*/\1/p' "$app"/VNA.cpp | head -1)
  [ -n "$tol" ] || { echo "could not read the 2.LO tolerance from VNA.cpp"; exit 1; }
  flags+=(-DLO2_TOL_DIV=$tol) ;;
esac
case " $* " in *" -DPATCH_LO2_ROUND "*)
  # round instead of truncate in the fast (non-exact) Si5351 divider path
  sed -i 's|best_b = (uint64_t) f_rem \* best_c / f;|best_b = ((uint64_t) f_rem * best_c + f / 2) / f;|' "$b"/Si5351C.cpp
  grep -q "f / 2) / f" "$b"/Si5351C.cpp || { echo "PATCH_LO2_ROUND did not apply"; exit 1; } ;;
esac
# count Stern-Brocot iterations (cost of an exact divider search on the MCU)
sed -i -e '1i unsigned long alg_iters;' -e 's|while (b + d <= max_denom) {|while (b + d <= max_denom) { ++alg_iters;|' "$b"/algorithm.cpp
g++ -std=c++17 -O2 -Wall -Wno-unused-variable -Wno-unused-function -Wno-sign-compare -I"$b" "${flags[@]}" "$@" \
    -o "$out" "$here"/fwplan.cpp "$b"/Si5351C.cpp "$b"/max2871.cpp "$b"/algorithm.cpp
echo "built $out from $(git -C "$repo" describe --tags --always 2>/dev/null) ${flags[*]} $*"
