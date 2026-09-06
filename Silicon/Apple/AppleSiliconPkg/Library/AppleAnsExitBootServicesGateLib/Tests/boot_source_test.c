/* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <Library/AppleAnsBootSource.h>
#include "../AppleAnsExitBootServicesGateLib.c"
UINT64 gTestPreloadBase;
UINT32 gTestPreloadSize = 0x10000;
STATIC UINT8 Header[16], Path[256];
STATIC UINTN PathBytes;
UINTN GetDevicePathSize (CONST EFI_DEVICE_PATH_PROTOCOL *P) { (VOID)P; return PathBytes; }
BOOLEAN IsDevicePathValid (CONST EFI_DEVICE_PATH_PROTOCOL *P, UINTN Max)
{ return P != NULL && PathBytes <= Max; }
VOID MemoryFence (VOID) {}
VOID CpuDeadLoop (VOID) { assert (0); }
UINT64 MmioRead64 (UINTN A) { (VOID)A; assert (0); return 0; }
UINT64 MmioWrite64 (UINTN A, UINT64 V) { (VOID)A; (VOID)V; assert (0); return 0; }
STATIC VOID Put (UINT8 *P, UINT64 V, UINTN N)
{ for (UINTN I = 0; I < N; ++I) { P[I] = (UINT8)V; V >>= 8; } }
STATIC VOID Fixture (CONST CHAR8 *File)
{
  CONST EFI_GUID Guid = EFI_VIRTUAL_DISK_GUID;
  memset (Header, 0, sizeof Header); memcpy (Header, "ASIRAMDK", 8); Put (Header+8, 0x8000, 8);
  gTestPreloadBase = (UINTN)Header; gTestPreloadSize = 0x10000;
  memset (Path, 0, sizeof Path);
  Path[0] = 4; Path[1] = 9; Put (Path+2, 38, 2);
  Put (Path+4, gTestPreloadBase+0x1000, 8); Put (Path+12, gTestPreloadBase+0x8fff, 8);
  memcpy (Path+20, &Guid, 16); PathBytes = 38;
  if (File) {
    UINTN N = strlen(File); Path[38] = 4; Path[39] = 4; Put (Path+40, 4+(N+1)*2, 2);
    for (UINTN I=0; I<N; ++I) Path[42+I*2]=(UINT8)File[I];
    PathBytes += 4+(N+1)*2;
  }
  Path[PathBytes]=0x7f;Path[PathBytes+1]=0xff;Path[PathBytes+2]=4;PathBytes+=4;
  mApprovedBootImage = NULL;
}
STATIC BOOLEAN Allowed (BOOLEAN File)
{ return AppleAnsBootSourceMatches(Path,PathBytes,gTestPreloadBase,gTestPreloadSize,Header,File); }
int main(void)
{
  Fixture("\\EFI\\BOOT\\BOOTAA64.EFI"); assert(Allowed(TRUE));
  assert(!Allowed(FALSE));
  assert(DxeStartImageGate((EFI_HANDLE)1,10,(VOID*)Path)==EFI_SUCCESS);
  assert(mApprovedBootImage==(EFI_HANDLE)1);
  assert(DxeExitBootServicesGate((EFI_HANDLE)2)==EFI_SECURITY_VIOLATION); /* No MMIO */
  DxeStartImageGateEnd((EFI_HANDLE)2);assert(mApprovedBootImage==(EFI_HANDLE)1);
  DxeStartImageGateEnd((EFI_HANDLE)1);assert(!mApprovedBootImage);
  assert(DxeExitBootServicesGate((EFI_HANDLE)1)==EFI_SECURITY_VIOLATION);
  Fixture("\\efi\\microsoft\\boot\\bootmgfw.efi");assert(Allowed(TRUE));
  Fixture(NULL);assert(Allowed(FALSE));assert(!Allowed(TRUE));
  Fixture("\\SHELL.EFI");assert(!Allowed(TRUE));
  assert(DxeStartImageGate((EFI_HANDLE)1,10,(VOID*)Path)==EFI_SECURITY_VIOLATION);
  for(unsigned Fault=0;Fault<12;++Fault) {
    Fixture("\\EFI\\BOOT\\BOOTAA64.EFI");
    switch(Fault) {
      case 0: Header[0]^=1;break;
      case 1: Put(Header+8,0,8);break;
      case 2: Put(Header+8,0x10000,8);break;
      case 3: Path[4]^=1;break;
      case 4: Path[12]^=1;break;
      case 5: Path[20]^=1;break;
      case 6: Path[0]=1;Path[1]=1;break; /* PCI/HD ancestor is not RAMDisk. */
      case 7: Path[PathBytes-3]=1;break; /* end-instance */
      case 8: Path[2]=0;break;
      case 9: PathBytes--;break;
      case 10: gTestPreloadSize=0x1000;break;
      case 11: gTestPreloadBase=MAX_UINT64-0x100;break;
    }
    assert(!Allowed(TRUE));
    assert(DxeStartImageGate((EFI_HANDLE)1,10,(VOID*)Path)==EFI_SECURITY_VIOLATION);
    assert(!mApprovedBootImage);
  }
  Fixture("\\EFI\\BOOT\\BOOTAA64.EFI");gTestPreloadBase=0;
  assert(!AppleAnsPreloadedBootPath((VOID*)Path,TRUE));
  assert(DxeStartImageGate((EFI_HANDLE)2,11,NULL)==EFI_SUCCESS);
  assert(DxeStartImageGate((EFI_HANDLE)3,12,NULL)==EFI_SUCCESS);
  assert(DxeStartImageGate((EFI_HANDLE)4,16,(VOID*)Path)==EFI_SECURITY_VIOLATION);
  /* Partition node appended by FAT/PartitionDxe remains on the exact RAMDisk. */
  Fixture("\\EFI\\BOOT\\BOOTAA64.EFI");
  memmove(Path+80,Path+38,PathBytes-38);memset(Path+38,0,42);Path[38]=4;Path[39]=1;Path[40]=42;PathBytes+=42;
  assert(Allowed(TRUE));
  puts("ANS qualification RAMDisk source/bootmgr/handle guard tests PASS");
  return 0;
}
