/* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <Uefi.h>
#include <Library/J313AppleAnsBroker.generated.h>
#include <Library/DxeExitBootServicesGateLib.h>
#ifndef ANS_TEST_NULL
#include "../AppleAnsExitBootServicesGateLib.c"
#endif
UINT64 gTestPreloadBase;
UINT32 gTestPreloadSize;
UINTN GetDevicePathSize (CONST EFI_DEVICE_PATH_PROTOCOL *P) { (VOID)P; return 0; }
BOOLEAN IsDevicePathValid (CONST EFI_DEVICE_PATH_PROTOCOL *P, UINTN M) { (VOID)P; (VOID)M; return FALSE; }
static union { struct ans_broker_header h; UINT64 q[6]; } header;
static union { struct ans_handoff_receipt r; UINT64 q[8]; } receipt;
static unsigned reads, writes, fences;
static int variant;
static jmp_buf terminal;
VOID MemoryFence (VOID) { ++fences; }
VOID CpuDeadLoop (VOID) { longjmp (terminal, 1); }
UINT64 MmioRead64 (UINTN address)
{
  ++reads;
  if (address >= ANS_BROKER_BASE && address < ANS_BROKER_BASE + 48) {
    UINT64 q = header.q[(address - ANS_BROKER_BASE) / 8];
    if (variant == 30 && reads == 7) q ^= 1;
    return q;
  }
  assert (address >= ANS_BROKER_BASE + ANS_HANDOFF_RECEIPT_OFFSET);
  assert (address < ANS_BROKER_BASE + ANS_HANDOFF_RECEIPT_OFFSET + 64);
  return receipt.q[(address - ANS_BROKER_BASE - ANS_HANDOFF_RECEIPT_OFFSET) / 8];
}
UINT64 MmioWrite64 (UINTN address, UINT64 sequence)
{
  assert (address == ANS_BROKER_BASE + ANS_HANDOFF_REQUEST_OFFSET);
  assert (reads == 12 && sequence != 0);
  ++writes;
  receipt.r = (struct ans_handoff_receipt){
    .magic = ANS_MAGIC, .abi_major = ANS_ABI_MAJOR, .abi_minor = ANS_ABI_MINOR,
    .size = 64, .result = ANS_RESULT_OK, .sequence = sequence,
    .previous_generation = header.h.generation, .generation = header.h.generation + 1,
    .proof_flags = ANS_HANDOFF_REQUIRED_PROOFS, .state = ANS_STATE_READY
  };
  if (variant >= 1 && variant <= 7) receipt.r.proof_flags &= ~(1u << (variant - 1));
  switch (variant) {
    case 8: receipt.r.sequence++; break;
    case 9: receipt.r.previous_generation++; break;
    case 10: receipt.r.generation++; break;
    case 11: receipt.r.abi_major++; break;
    case 12: receipt.r.abi_minor++; break;
    case 13: receipt.r.result = ANS_RESULT_IO_ERROR; break;
    case 14: receipt.r.state = ANS_STATE_FAULTED; break;
    case 15: receipt.r.magic++; break;
    case 16: receipt.r.size--; break;
    case 17: receipt.r.reserved[0] = 1; break;
    case 18: receipt.r.reserved[1] = 1; break;
    case 19: receipt.r.proof_flags |= 128; break;
  }
  return sequence;
}
static void reset (VOID)
{
  reads = writes = fences = 0; variant = 0;
  memset (&header, 0, sizeof (header));
  memset (&receipt, 0, sizeof (receipt));
  header.h.magic = ANS_MAGIC; header.h.abi_major = ANS_ABI_MAJOR;
  header.h.abi_minor = ANS_ABI_MINOR; header.h.size = 48; header.h.generation = 7;
#ifndef ANS_TEST_NULL
  mHandoffSequence = 0;
  mApprovedBootImage = (EFI_HANDLE)1;
#endif
}
int main (void)
{
  reset ();
  assert (DxeExitBootServicesGate ((EFI_HANDLE)1) == EFI_SUCCESS);
#ifdef ANS_TEST_NULL
  assert (!reads && !writes && !fences);
  assert (DxeStartImageGate (NULL, 10, NULL) == EFI_SUCCESS);
  DxeStartImageGateEnd (NULL);
#else
  assert (DxeExitBootServicesGate ((EFI_HANDLE)2) == EFI_SECURITY_VIOLATION);
  assert (reads == 20 && writes == 1 && fences == 2 && receipt.r.sequence == 1);
  reads = writes = 0;
  header.h.generation = 8;
  assert (DxeExitBootServicesGate ((EFI_HANDLE)1) == EFI_SUCCESS && receipt.r.sequence == 2);
  for (int bad = 1; bad <= 19; ++bad) {
    reset (); variant = bad;
    assert (DxeExitBootServicesGate ((EFI_HANDLE)1) == EFI_DEVICE_ERROR && writes == 1 && reads == 20);
    assert (gAppleAnsHandoffFailureStage == 4);
    for (unsigned i = 0; i < 8; ++i) assert (gAppleAnsHandoffReceiptWords[i] == receipt.q[i]);
  }
  reset (); variant = 30;
  assert (DxeExitBootServicesGate ((EFI_HANDLE)1) == EFI_NOT_READY && !writes);
  for (unsigned bad = 0; bad < 6; ++bad) {
    reset ();
    switch (bad) {
      case 0: header.h.magic++; break;
      case 1: header.h.abi_major++; break;
      case 2: header.h.abi_minor++; break;
      case 3: header.h.size--; break;
      case 4: header.h.reserved = 1; break;
      case 5: header.h.generation = MAX_UINT64; break;
    }
    assert (DxeExitBootServicesGate ((EFI_HANDLE)1) == EFI_INCOMPATIBLE_VERSION && !writes);
  }
  reset (); mHandoffSequence = MAX_UINT64;
  assert (DxeExitBootServicesGate ((EFI_HANDLE)1) == EFI_OUT_OF_RESOURCES && !writes);
#endif
  if (!setjmp (terminal)) {
    DxeExitBootServicesGateFailed (EFI_DEVICE_ERROR);
    assert (0);
  }
#ifndef ANS_TEST_NULL
  assert (gAppleAnsHandoffFailureStatus == EFI_DEVICE_ERROR);
#endif
  puts ("ANS EBS gate tests PASS");
  return 0;
}
