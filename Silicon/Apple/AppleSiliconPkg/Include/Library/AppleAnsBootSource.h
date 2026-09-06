/** @file Native qualification boot-source predicate; no installed-disk boot.
    SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT **/
#ifndef APPLE_ANS_BOOT_SOURCE_H_
#define APPLE_ANS_BOOT_SOURCE_H_
#include <Uefi.h>
#include <Protocol/DevicePath.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/PreloadedRamdisk.h>
#define APPLE_ANS_BOOT_PATH_MAX 1024
STATIC inline UINT64 AppleAnsBootLe (CONST UINT8 *P, UINTN N)
{
  UINT64 Value = 0;
  for (UINTN I = 0; I < N; ++I) Value |= (UINT64)P[I] << (I * 8);
  return Value;
}
STATIC inline BOOLEAN AppleAnsBootEqual (CONST UINT8 *A, CONST UINT8 *B, UINTN N)
{
  for (UINTN I = 0; I < N; ++I) if (A[I] != B[I]) return FALSE;
  return TRUE;
}
STATIC inline BOOLEAN AppleAnsBootFile (CONST UINT8 *P, UINTN N, CONST CHAR8 *Expected)
{
  UINTN I;
  for (I = 0; Expected[I]; ++I) {
    if (N < 2 || I >= (N / 2) - 1 || P[2 * I + 1]) return FALSE;
    UINT8 C = P[2 * I];
    if (C >= 'a' && C <= 'z') C -= 'a' - 'A';
    if (C != (UINT8)Expected[I]) return FALSE;
  }
  return N == (I + 1) * 2 && P[I * 2] == 0 && P[I * 2 + 1] == 0;
}
/* Core supplies the owned full LoadedImageDevicePath, not only FilePath.
 * UEFI2.10 9.2 specifies that copy; RamDiskDxe creates first MEDIA_RAM_DISK_DP
 * (parentNULL) with inclusive end. Optional partition follows it. No alternate
 * ancestor or multi-instance path may smuggle in internal storage. */
STATIC inline BOOLEAN AppleAnsBootSourceMatches (
  CONST VOID *DevicePath, UINTN Bytes, UINT64 HeaderBase, UINT64 MaximumBytes,
  CONST VOID *Header, BOOLEAN RequireWindowsFile)
{
  CONST UINT8 *P = DevicePath;
  CONST UINT8 *H = Header;
  CONST EFI_GUID DiskType = EFI_VIRTUAL_DISK_GUID;
  CONST UINT8 Magic[8] = PRELOADED_RAMDISK_MAGIC;
  UINTN Offset = 0;
  BOOLEAN Partition = FALSE, File = FALSE;
  if (!P || !H || Bytes < 42 || Bytes > APPLE_ANS_BOOT_PATH_MAX || !HeaderBase ||
      MaximumBytes <= PRELOADED_RAMDISK_PAYLOAD_OFFSET || HeaderBase > MAX_UINT64 - MaximumBytes ||
      !AppleAnsBootEqual (H, Magic, 8)) return FALSE;
  UINT64 Size = AppleAnsBootLe (H + 8, 8);
  UINT64 Start = HeaderBase + PRELOADED_RAMDISK_PAYLOAD_OFFSET;
  if (!Size || Size > MaximumBytes - PRELOADED_RAMDISK_PAYLOAD_OFFSET ||
      P[0] != MEDIA_DEVICE_PATH || P[1] != MEDIA_RAM_DISK_DP || AppleAnsBootLe (P + 2, 2) != 38 ||
      AppleAnsBootLe (P + 4, 8) != Start || AppleAnsBootLe (P + 12, 8) != Start + Size - 1 ||
      !AppleAnsBootEqual (P + 20, (CONST UINT8 *)&DiskType, 16)) return FALSE;
  Offset = 38;
  while (Offset < Bytes) {
    if (Bytes - Offset < 4) return FALSE;
    CONST UINT8 *Node = P + Offset;
    UINTN Length = (UINTN)AppleAnsBootLe (Node + 2, 2);
    if (Length < 4 || Length > Bytes - Offset) return FALSE;
    if (Node[0] == END_DEVICE_PATH_TYPE) {
      return Node[1] == END_ENTIRE_DEVICE_PATH_SUBTYPE && Length == 4 &&
             Offset + 4 == Bytes && File == RequireWindowsFile;
    }
    if (Node[0] != MEDIA_DEVICE_PATH) return FALSE;
    if (Node[1] == MEDIA_HARDDRIVE_DP && !Partition && !File && Length == 42) {
      Partition = TRUE;
    } else if (Node[1] == MEDIA_FILEPATH_DP && RequireWindowsFile && !File &&
               (AppleAnsBootFile (Node + 4, Length - 4, "\\EFI\\BOOT\\BOOTAA64.EFI") ||
                AppleAnsBootFile (Node + 4, Length - 4, "\\EFI\\MICROSOFT\\BOOT\\BOOTMGFW.EFI"))) {
      File = TRUE;
    } else return FALSE;
    Offset += Length;
  }
  return FALSE;
}
/* Call only before EBS validation. PCD describes a reserved guest-RAM window;
 * this predicate performs no physical ANS access, allocation, or registration. */
STATIC inline BOOLEAN AppleAnsPreloadedBootPath (CONST EFI_DEVICE_PATH_PROTOCOL *Path,
                                                BOOLEAN RequireWindowsFile)
{
  UINT64 Base = PcdGet64 (PcdPreloadedRamdiskBase);
  UINT64 Size = PcdGet32 (PcdPreloadedRamdiskMaxSize);
  if (!Base || Size <= PRELOADED_RAMDISK_PAYLOAD_OFFSET || Base > MAX_UINT64 - Size ||
      !Path || !IsDevicePathValid (Path, APPLE_ANS_BOOT_PATH_MAX)) return FALSE;
  return AppleAnsBootSourceMatches (Path, GetDevicePathSize (Path), Base, Size,
                                   (CONST VOID *)(UINTN)Base, RequireWindowsFile);
}
#endif
