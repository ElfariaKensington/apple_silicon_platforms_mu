#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause-Patent
# ACPI source-only check. No firmware image, launch, or hardware access.
set -eu
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mu=$(CDPATH= cd -- "$here/../../../../../.." && pwd)
out="$mu/.local/ans-native-ebs-tests"
mkdir -p "$out"
docker run --rm --name "ans-native-acpi-check-$$" \
  --mount "type=bind,source=$mu,target=/ans,readonly" \
  --mount "type=bind,source=$out,target=/out" \
  windows-on-m1-build:local /bin/sh -c '
set -eu
export PYTHONPATH=/ans/MU_BASECORE/BaseTools/Source/Python
trim=/ans/MU_BASECORE/BaseTools/Source/Python/Trim/Trim.py
source_dir=/ans/Platform/MacBookAirMid2020Pkg/AcpiTables
for name in DSDT DSDTAnsNative J313AppleAgxSsdt; do
  python3 "$trim" --asl-file -o "/out/$name.i" "$source_dir/$name.asl"
  cpp -I/ans/MU_BASECORE/MdePkg/Include "/out/$name.i" > "/out/$name.pp"
  python3 "$trim" --source-code -l -o "/out/$name.asl" "/out/$name.pp"
  cd /out
  iasl -p "$name" "$name.asl"
  test -s "/out/$name.aml"
done
'
