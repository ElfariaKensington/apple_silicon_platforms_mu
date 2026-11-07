/** @file
  Opt-in J313 synchronous EL2 ownership handoff, offline candidate only.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/DxeExitBootServicesGateLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/J313AppleAnsBroker.generated.h>
#include <Library/AppleAnsBootSource.h>
#include <IndustryStandard/PeImage.h>

/* Diagnostic-only preserved state, no protocols, prints, allocation or reset. */
volatile EFI_STATUS  gAppleAnsHandoffFailureStatus;
volatile UINT32      gAppleAnsHandoffFailureStage;
volatile UINT64      gAppleAnsHandoffReceiptWords[8];
STATIC UINT64        mHandoffSequence;
STATIC EFI_HANDLE    mApprovedBootImage;

STATIC EFI_STATUS
Fail (UINT32 Stage, EFI_STATUS Status)
{
  gAppleAnsHandoffFailureStage = Stage;
  gAppleAnsHandoffFailureStatus = Status;
  return Status;
}

EFI_STATUS EFIAPI
DxeExitBootServicesGate (IN EFI_HANDLE ImageHandle)
{
  union { struct ans_broker_header Header; UINT64 Words[6]; } First, Second;
  union { struct ans_handoff_receipt Receipt; UINT64 Words[8]; } Snapshot;
  UINTN Index;
  UINT64 PreviousGeneration;
  UINT64 Sequence;

  if (!ImageHandle || ImageHandle != mApprovedBootImage) {
    return Fail (5, EFI_SECURITY_VIOLATION);
  }

  /* Two equal full snapshots: no unbounded retry on a moving header. EL2 must
     serialize this synchronous handoff trap and subsequent receipt reads. */
  for (Index = 0; Index < 6; ++Index) {
    First.Words[Index] = MmioRead64 ((UINTN)ANS_BROKER_BASE + Index * sizeof (UINT64));
  }
  MemoryFence ();
  for (Index = 0; Index < 6; ++Index) {
    Second.Words[Index] = MmioRead64 ((UINTN)ANS_BROKER_BASE + Index * sizeof (UINT64));
    if (First.Words[Index] != Second.Words[Index]) {
      return Fail (1, EFI_NOT_READY);
    }
  }
  if ((First.Header.magic != ANS_MAGIC) ||
      (First.Header.abi_major != ANS_ABI_MAJOR) ||
      (First.Header.abi_minor != ANS_ABI_MINOR) ||
      (First.Header.size != sizeof (First.Header)) || First.Header.reserved ||
      (First.Header.generation == MAX_UINT64)) {
    return Fail (2, EFI_INCOMPATIBLE_VERSION);
  }
  if (mHandoffSequence == MAX_UINT64) {
    return Fail (3, EFI_OUT_OF_RESOURCES);
  }
  PreviousGeneration = First.Header.generation;
  Sequence = ++mHandoffSequence;
  /* Exactly one synthetic doorbell write. Its eventual EL2 handler must be
     bounded and synchronous; it owns drain/flush/disable/RTKit/PCI/arm proof. */
  MmioWrite64 ((UINTN)ANS_BROKER_BASE + ANS_HANDOFF_REQUEST_OFFSET, Sequence);
  MemoryFence ();
  for (Index = 0; Index < 8; ++Index) {
    Snapshot.Words[Index] = MmioRead64 (
      (UINTN)ANS_BROKER_BASE + ANS_HANDOFF_RECEIPT_OFFSET + Index * sizeof (UINT64));
    gAppleAnsHandoffReceiptWords[Index] = Snapshot.Words[Index];
  }
  if (!ans_handoff_receipt_valid (&Snapshot.Receipt, Sequence, PreviousGeneration)) {
    return Fail (4, EFI_DEVICE_ERROR);
  }
  gAppleAnsHandoffFailureStage = 0;
  gAppleAnsHandoffFailureStatus = EFI_SUCCESS;
  return EFI_SUCCESS;
}

VOID EFIAPI
DxeExitBootServicesGateFailed (IN EFI_STATUS Status)
{
  gAppleAnsHandoffFailureStatus = Status;
  MemoryFence ();
  CpuDeadLoop ();
}

EFI_STATUS EFIAPI
DxeStartImageGate (IN EFI_HANDLE ImageHandle, IN UINT16 Subsystem,
                  IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  if (Subsystem == EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER ||
      Subsystem == EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER) return EFI_SUCCESS;
  if (!ImageHandle || Subsystem != EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION ||
      mApprovedBootImage || !AppleAnsNativeBootPath (DevicePath, TRUE))
    return Fail (6, EFI_SECURITY_VIOLATION);
  mApprovedBootImage = ImageHandle;
  return EFI_SUCCESS;
}
VOID EFIAPI
DxeStartImageGateEnd (IN EFI_HANDLE ImageHandle)
{
  if (ImageHandle == mApprovedBootImage) mApprovedBootImage = NULL;
}
