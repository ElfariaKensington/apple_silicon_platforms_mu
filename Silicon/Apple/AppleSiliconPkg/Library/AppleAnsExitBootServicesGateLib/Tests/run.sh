#!/bin/sh
set -eu
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mu=$(CDPATH= cd -- "$here/../../../../../.." && pwd)
build=$(mktemp -d "${TMPDIR:-/tmp}/ans-ebs.XXXXXXXX")
trap 'rm -rf "$build"' EXIT HUP INT TERM
python3 - "$mu" "$build" <<'PY'
from pathlib import Path
import sys
mu, out = map(Path, sys.argv[1:])
s = (mu/'MU_BASECORE/MdeModulePkg/Core/Dxe/DxeMain/DxeMain.c').read_text()
start = s.index('EFI_STATUS\nEFIAPI\nCoreExitBootServices (')
end = s.index('  CoreNotifySignalList (&gEfiEventExitBootServicesGuid);', start)
body = s[start:end+len('  CoreNotifySignalList (&gEfiEventExitBootServicesGuid);')]
(out/'core_boundary.h').write_text(body+'\n  (VOID)ImageHandle;\n  return EFI_SUCCESS;\n}\n')
src=(mu/'Silicon/Apple/AppleSiliconPkg/Drivers/WindowsAutoBootDxe/WindowsAutoBootDxe.c').read_text()
parts=[]
for name,ret in [('ConnectAllBlockIoHandles','VOID'),('TryStartWindowsFromHandle','BOOLEAN'),('WindowsAutoBootReadyToBoot','VOID')]:
 a=src.index(name+' ('); b=src.index('\n}',a)+2
 parts.append('static '+ret+'\n'+src[a:b])
(out/'autoboot_functions.h').write_text('\n'.join(parts))
src=(mu/'MU_BASECORE/MdeModulePkg/Core/Dxe/Image/Image.c').read_text()
a=src.index('EFI_STATUS\nEFIAPI\nCoreStartImage ('); b=src.index('  PERF_START_IMAGE_BEGIN (Handle);',a)
(out/'start_boundary.h').write_text(src[a:b]+'\n (VOID)ExitDataSize; (VOID)ExitData; (VOID)LastImage; (VOID)HandleDatabaseKey; (VOID)SetJumpFlag; (VOID)Handle;\n return EFI_SUCCESS;\n}\n')

PY
for kind in native null core source installed autoboot start; do
  if [ "$kind" = autoboot ]; then
    set -- "$here/autoboot_source_test.c" "-DANS_TEST_AUTOBOOT_FUNCTIONS=\"$build/autoboot_functions.h\""
  elif [ "$kind" = start ]; then
    set -- "$here/start_image_boundary_test.c" "-DANS_TEST_START_BOUNDARY=\"$build/start_boundary.h\""
  elif [ "$kind" = source ]; then
    set -- "$here/boot_source_test.c"
  elif [ "$kind" = installed ]; then
    set -- "$here/installed_source_test.c"
  elif [ "$kind" = core ]; then
    set -- "$here/core_boundary_test.c" "-DANS_TEST_CORE_BOUNDARY=\"$build/core_boundary.h\""
  elif [ "$kind" = null ]; then
    set -- "$here/gate_test.c" -DANS_TEST_NULL "$mu/MU_BASECORE/MdeModulePkg/Library/DxeExitBootServicesGateLibNull/DxeExitBootServicesGateLibNull.c"
  else
    set -- "$here/gate_test.c"
  fi
  "${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer -I"$here/Include" \
    -I"$mu/MU_BASECORE/MdeModulePkg/Include" \
    -I"$mu/Silicon/Apple/AppleSiliconPkg/Include" "$@" -o "$build/$kind"
  "$build/$kind"
done
