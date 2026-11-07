/* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <Uefi.h>
static BOOLEAN mExitBootServicesCalled, gMemoryMapTerminated;
static int gEfiEventBeforeExitBootServicesGuid, gEventExitBootServicesFailedGuid;
static int gEfiEventExitBootServicesGuid;
static unsigned before, failed, exited, gated, timer;
static EFI_STATUS map_status, gate_status, terminal_status;
static jmp_buf stop;
struct fake_timer { VOID (*SetTimerPeriod)(struct fake_timer *, UINTN); };
static VOID SetPeriod(struct fake_timer *self, UINTN period)
{ (VOID)self; assert (!period); ++timer; }
static struct fake_timer timer_instance = {SetPeriod};
static struct fake_timer *gTimer = &timer_instance;
static VOID CoreNotifySignalList(int *event)
{
  if (event == &gEfiEventBeforeExitBootServicesGuid) ++before;
  else if (event == &gEventExitBootServicesFailedGuid) ++failed;
  else { assert (event == &gEfiEventExitBootServicesGuid && gMemoryMapTerminated); ++exited; }
}
static EFI_STATUS CoreTerminateMemoryMap(UINTN key)
{ assert (key == 77 && timer && !gMemoryMapTerminated); return map_status; }
static EFI_STATUS DxeExitBootServicesGate(EFI_HANDLE ImageHandle)
{ (VOID)ImageHandle; assert (!EFI_ERROR(map_status) && !gMemoryMapTerminated && !exited); ++gated; return gate_status; }
static VOID DxeExitBootServicesGateFailed(EFI_STATUS status)
{ terminal_status = status; /* Deliberately returns: test defensive fallback. */ }
static VOID CpuDeadLoop(VOID) { longjmp (stop, 1); }
#include ANS_TEST_CORE_BOUNDARY
static void reset(void)
{
  mExitBootServicesCalled = gMemoryMapTerminated = FALSE;
  before = failed = exited = gated = timer = 0;
  map_status = gate_status = terminal_status = EFI_SUCCESS;
}
int main(void)
{
  reset(); map_status = EFI_INVALID_PARAMETER;
  assert (CoreExitBootServices(NULL,77) == EFI_INVALID_PARAMETER);
  assert (before == 1 && failed == 1 && !gated && !exited && !gMemoryMapTerminated);
  map_status = EFI_SUCCESS;
  assert (CoreExitBootServices(NULL,77) == EFI_SUCCESS);
  assert (before == 1 && gated == 1 && exited == 1 && gMemoryMapTerminated);
  reset(); gate_status = EFI_DEVICE_ERROR;
  if (!setjmp(stop)) { (VOID)CoreExitBootServices(NULL,77); assert(0); }
  assert (terminal_status == EFI_DEVICE_ERROR && gated == 1 && !failed && !exited && !gMemoryMapTerminated);
  puts("Actual CoreExitBootServices boundary tests PASS");
  return 0;
}
