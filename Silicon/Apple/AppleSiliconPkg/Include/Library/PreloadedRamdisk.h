/** SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT **/
#ifndef APPLE_PRELOADED_RAMDISK_H_
#define APPLE_PRELOADED_RAMDISK_H_
#include <Uefi.h>
/* Existing run_uefi.py / BootRamdiskHelperDxe ABI, moved without value changes. */
#define PRELOADED_RAMDISK_MAGIC           "ASIRAMDK"
#define PRELOADED_RAMDISK_PAYLOAD_OFFSET  0x1000
#pragma pack(1)
typedef struct {
  CHAR8 Magic[8];
  UINT64 Size;
} PRELOADED_RAMDISK_HEADER;
#pragma pack()
#endif
