/* SPDX-License-Identifier: BSD-2-Clause-Patent */
/* Installed native boot: the gate admits only the pinned Windows ESP on the emulated NVMe. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define APPLE_ANS_INSTALLED_ONLY 1
#include <Library/AppleAnsBootSource.h>
#include "../AppleAnsExitBootServicesGateLib.c"
UINT64 gTestPreloadBase;
UINT32 gTestPreloadSize = 0x10000;
UINT64 gTestEspStart, gTestEspBlocks;
UINT8 gTestEspGuid[16];
STATIC UINT8 Path[256];
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
/* EXP2225: PciRoot(0x0)/Pci(0x0,0x0)/NVMe(0x1,00-..)/HD(5,GPT,F05B9952-...,0x1D5AF00,0x20000)/\EFI\BOOT\BOOTAA64.EFI */
STATIC CONST UINT8 Winesp[16] = {0x52,0x99,0x5B,0xF0,0x05,0xEE,0xE1,0x44,0xBC,0xC4,0xB6,0x78,0xA8,0x50,0x1B,0x2A};
STATIC VOID Fixture (CONST CHAR8 *File)
{
  gTestEspStart = 0x1D5AF00; gTestEspBlocks = 0x20000; memcpy (gTestEspGuid, Winesp, 16);
  memset (Path, 0, sizeof Path);
  UINT8 *P = Path;
  P[0]=2;P[1]=1;Put(P+2,12,2);Put(P+4,0x0A0341D0,4);Put(P+8,0,4);           /* PciRoot(0) */
  P[12]=1;P[13]=1;Put(P+14,6,2);P[16]=0;P[17]=0;                            /* Pci(0,0) */
  P[18]=3;P[19]=0x17;Put(P+20,16,2);Put(P+22,1,4);                          /* NVMe(1,EUI 0) */
  UINT8 *H=P+34;H[0]=4;H[1]=1;Put(H+2,42,2);Put(H+4,5,4);Put(H+8,0x1D5AF00,8);Put(H+16,0x20000,8);
  memcpy(H+24,Winesp,16);H[40]=2;H[41]=2;                                    /* HD(5,GPT,...) */
  PathBytes = 76;
  if (File) {
    UINTN N = strlen(File); Path[76]=4; Path[77]=4; Put(Path+78, 4+(N+1)*2, 2);
    for (UINTN I=0; I<N; ++I) Path[80+I*2]=(UINT8)File[I];
    PathBytes += 4+(N+1)*2;
  }
  Path[PathBytes]=0x7f;Path[PathBytes+1]=0xff;Path[PathBytes+2]=4;PathBytes+=4;
  mApprovedBootImage = NULL;
}
STATIC BOOLEAN Allowed (BOOLEAN File) { return AppleAnsNativeBootPath ((VOID *)Path, File); }
int main(void)
{
  Fixture("\\EFI\\BOOT\\BOOTAA64.EFI"); assert(Allowed(TRUE)); assert(!Allowed(FALSE));
  assert(DxeStartImageGate((EFI_HANDLE)1,10,(VOID*)Path)==EFI_SUCCESS && mApprovedBootImage==(EFI_HANDLE)1);
  assert(DxeStartImageGate((EFI_HANDLE)2,10,(VOID*)Path)==EFI_SECURITY_VIOLATION); /* one approved loader */
  DxeStartImageGateEnd((EFI_HANDLE)1); assert(!mApprovedBootImage);
  Fixture("\\efi\\microsoft\\boot\\bootmgfw.efi"); assert(Allowed(TRUE));
  Fixture(NULL); assert(Allowed(FALSE)); assert(!Allowed(TRUE));            /* SimpleFS handle path */
  Fixture("\\EFI\\BOOT\\SHELL.EFI"); assert(!Allowed(TRUE));
  assert(DxeStartImageGate((EFI_HANDLE)1,10,(VOID*)Path)==EFI_SECURITY_VIOLATION);
  for (unsigned Fault = 0; Fault < 20; ++Fault) {
    Fixture("\\EFI\\BOOT\\BOOTAA64.EFI");
    switch (Fault) {
      case 0: Put(Path+4,0x0A0541D0,4); break;      /* not a PCI root bridge */
      case 1: Put(Path+8,1,4); break;               /* PciRoot(1) */
      case 2: Path[16]=1; break;                    /* Pci(0,1) */
      case 3: Path[17]=1; break;                    /* Pci(1,0) */
      case 4: Put(Path+22,2,4); break;              /* NVMe namespace 2 */
      case 5: Path[19]=0x12; break;                 /* SATA instead of NVMe */
      case 6: Put(Path+42,0x1D5AF01,8); break;      /* HD start */
      case 7: Put(Path+50,0x20001,8); break;        /* HD size */
      case 8: Path[58]^=1; break;                   /* partition GUID */
      case 9: Path[74]=1; break;                    /* MBR partitioning */
      case 10: Path[75]=1; break;                   /* MBR signature */
      case 11: Put(Path+38,0,4); break;             /* partition number 0 = whole disk */
      case 12: gTestEspStart=0; break;              /* unset PCDs: no installed boot */
      case 13: gTestEspBlocks=0; break;
      case 14: gTestEspGuid[0]^=1; break;
      case 15: Path[PathBytes-3]=1; break;          /* end-instance */
      case 16: PathBytes--; break;
      case 17: Path[0]=4; Path[1]=9; break;         /* RAMDisk ancestor is not admitted here */
      case 18: Put(Path+2,13,2); break;             /* ACPI node length */
      case 19: gTestEspStart=MAX_UINT64; break;     /* overflow guard */
    }
    assert(!Allowed(TRUE));
    assert(DxeStartImageGate((EFI_HANDLE)1,10,(VOID*)Path)==EFI_SECURITY_VIOLATION);
    assert(!mApprovedBootImage);
  }
  /* The PNP0A08 spelling of the same root bridge is accepted. */
  Fixture("\\EFI\\BOOT\\BOOTAA64.EFI"); Put(Path+4,0x0A0841D0,4); assert(Allowed(TRUE));
  /* An additional node between HD and the loader is refused. */
  Fixture("\\EFI\\BOOT\\BOOTAA64.EFI");
  memmove(Path+80,Path+76,PathBytes-76); Path[76]=4;Path[77]=1;Put(Path+78,4,2); PathBytes+=4;
  assert(!Allowed(TRUE));
  puts("ANS installed-boot ESP source/bootmgr/handle guard tests PASS");
  return 0;
}
