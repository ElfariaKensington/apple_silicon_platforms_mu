/** @file Native boot-source predicates: preloaded WinPE RAMDisk, or (installed profile only) one pinned ESP.
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
  CONST UINT8 Magic[] = PRELOADED_RAMDISK_MAGIC; /* 8 bytes compared; NUL unused */
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
/* Installed native boot (J313_ANS_INSTALLED_BOOT): only the one pinned Windows ESP on the
 * pre-EBS emulated NVMe bridge. Shape observed on hardware (recovery EXP2225 firmware log):
 * PciRoot(0x0)/Pci(0x0,0x0)/NVMe(0x1,EUI)/HD(n,GPT,<guid>,start,size)[/loader]/End.
 * ACPI 12 + PCI 6 + NVMe 16 + HD 42 bytes; HD start/size/signature are pinned, the GPT
 * entry number and EUI are not (they do not identify the partition). */
STATIC inline BOOLEAN AppleAnsInstalledBootSourceMatches (
  CONST VOID *DevicePath, UINTN Bytes, UINT64 EspStart, UINT64 EspBlocks,
  CONST UINT8 *EspGuid, BOOLEAN RequireWindowsFile)
{
  CONST UINT8 *P = DevicePath;
  CONST UINT8 *H;
  UINT64 Hid;
  UINTN Offset;
  BOOLEAN File = FALSE;
  if (!P || !EspGuid || !EspStart || !EspBlocks || EspStart > MAX_UINT64 - EspBlocks ||
      Bytes < 12 + 6 + 16 + 42 + 4 || Bytes > APPLE_ANS_BOOT_PATH_MAX) return FALSE;
  Hid = AppleAnsBootLe (P + 4, 4);
  if (P[0] != ACPI_DEVICE_PATH || P[1] != ACPI_DP || AppleAnsBootLe (P + 2, 2) != 12 ||
      (Hid != 0x0A0341D0 && Hid != 0x0A0841D0) || AppleAnsBootLe (P + 8, 4) != 0) return FALSE;
  if (P[12] != HARDWARE_DEVICE_PATH || P[13] != HW_PCI_DP || AppleAnsBootLe (P + 14, 2) != 6 ||
      P[16] != 0 || P[17] != 0) return FALSE;
  if (P[18] != MESSAGING_DEVICE_PATH || P[19] != MSG_NVME_NAMESPACE_DP ||
      AppleAnsBootLe (P + 20, 2) != 16 || AppleAnsBootLe (P + 22, 4) != 1) return FALSE;
  H = P + 34;
  if (H[0] != MEDIA_DEVICE_PATH || H[1] != MEDIA_HARDDRIVE_DP || AppleAnsBootLe (H + 2, 2) != 42 ||
      !AppleAnsBootLe (H + 4, 4) || AppleAnsBootLe (H + 8, 8) != EspStart ||
      AppleAnsBootLe (H + 16, 8) != EspBlocks || !AppleAnsBootEqual (H + 24, EspGuid, 16) ||
      H[40] != 2 /* MBR_TYPE_EFI_PARTITION_TABLE_HEADER */ || H[41] != 2 /* SIGNATURE_TYPE_GUID */)
    return FALSE;
  Offset = 76;
  while (Offset < Bytes) {
    if (Bytes - Offset < 4) return FALSE;
    CONST UINT8 *Node = P + Offset;
    UINTN Length = (UINTN)AppleAnsBootLe (Node + 2, 2);
    if (Length < 4 || Length > Bytes - Offset) return FALSE;
    if (Node[0] == END_DEVICE_PATH_TYPE) {
      return Node[1] == END_ENTIRE_DEVICE_PATH_SUBTYPE && Length == 4 &&
             Offset + 4 == Bytes && File == RequireWindowsFile;
    }
    if (Node[0] == MEDIA_DEVICE_PATH && Node[1] == MEDIA_FILEPATH_DP && RequireWindowsFile && !File &&
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
#ifdef APPLE_ANS_INSTALLED_ONLY
/* The pinned ESP comes from FixedAtBuild PCDs set only by the installed profile DSC. */
STATIC inline BOOLEAN AppleAnsInstalledBootPath (CONST EFI_DEVICE_PATH_PROTOCOL *Path,
                                                BOOLEAN RequireWindowsFile)
{
  if (!Path || !IsDevicePathValid (Path, APPLE_ANS_BOOT_PATH_MAX)) return FALSE;
  return AppleAnsInstalledBootSourceMatches (Path, GetDevicePathSize (Path),
                                            PcdGet64 (PcdAnsInstalledEspStartLba),
                                            PcdGet64 (PcdAnsInstalledEspBlocks),
                                            (CONST UINT8 *)PcdGetPtr (PcdAnsInstalledEspGuid),
                                            RequireWindowsFile);
}
#define AppleAnsNativeBootPath AppleAnsInstalledBootPath
#else
#define AppleAnsNativeBootPath AppleAnsPreloadedBootPath
#endif
#endif
