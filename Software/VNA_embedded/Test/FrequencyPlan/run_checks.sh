#!/bin/bash
# Regression check of the VNA frequency plan against one firmware tree.
#   run_checks.sh FIRMWARE_REPO [LIMIT] [extra build flags]
# Builds fwplan from the tree's own drivers and replays a set of standard sweeps. A sweep fails
# if any point's true final-IF offset exceeds LIMIT x IFBW (default 0.5; 0.5 x IFBW costs about 1 dB and 1.0 x IFBW about 4 dB
# of DFT loss with the FPGA's Kaiser window). Exit status 1 if any sweep fails.
#   v1.6.4: fails (offsets up to 24 x IFBW).   v1.6.5 / master 13f37c5: 6 sweeps fail at 0.5 (worst 0.87 x IFBW).
here=$(cd "$(dirname "$0")" && pwd)
repo=$1; limit=${2:-0.5}; shift; shift
bin=$(mktemp -d)/fwplan
bash "$here"/build.sh "$repo" "$bin" "$@" >/dev/null || exit 2
fail=0
while read -r f0 f1 n bw opt; do
  "$bin" $f0 $f1 $n $bw $opt --limit=$limit || fail=1
done <<'LIST'
1000000 6000000000 501 10
1000000 6000000000 501 6
1000000 6000000000 501 100
1000000 6000000000 501 1000
1000000 6000000000 4501 10
1000000 6000000000 4501 100
1000000 6000000000 4501 1000
1000000 6000000000 501 10 log
100000 30000000 201 10
1750500000 2050500000 31 10
2780000000 2980000000 501 10
5640000000 5840000000 501 10
4500000000 6000000000 1501 10
LIST
rm -rf "$(dirname "$bin")"
exit $fail
