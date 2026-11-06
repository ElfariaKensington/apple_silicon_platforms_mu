/** @file
  J313: plan EfiReservedMemoryType reservations for iBoot carveouts inside guest RAM.

  /chosen/carveout-memory-map lists firmware regions ("region-id-N" = {base, size}).
  Thirteen of them lie inside the J313 guest RAM window and have no named owner; the
  m1n1 hypervisor can remove them from stage-2 (WOM1_HV_UNMAP_RAM_CARVEOUTS), so the OS
  must never be given those pages.  This planner is the pure part of that reservation:

    * a carveout covering the whole RAM window is the iBoot kernel-DRAM descriptor
      (region-id-24 on J313), not a reservation, and is ignored;
    * the rest are clipped to RAM, widened to 16 KiB pages and merged when separated by
      at most RAM_CARVEOUT_MERGE_GAP bytes -- the same rule as m1n1's
      stage2_policy.in_ram_carveouts(), so reserved >= unmapped;
    * ranges the platform already owns (framebuffer, a reserved RAM-disk window, the
      low-memory backing, the AGX local reserve) are subtracted, so nothing is reserved
      twice and the existing reservations stay exactly as they are;
    * overlap with a range marked Fatal (the firmware volume, UEFI permanent memory)
      fails the plan: firmware already lives there and cannot be withdrawn.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef RAM_CARVEOUT_RESERVATION_H_
#define RAM_CARVEOUT_RESERVATION_H_

#include <Base.h>

#define RAM_CARVEOUT_PAGE        0x4000ULL
#define RAM_CARVEOUT_MERGE_GAP   0x4000ULL
#define RAM_CARVEOUT_MAX_ID      255U
#define RAM_CARVEOUT_MAX_INPUT   32U
#define RAM_CARVEOUT_MAX_RANGES  32U

typedef struct {
  UINT64  Base;
  UINT64  Size;
} RAM_CARVEOUT_RANGE;

typedef struct {
  UINT64   Base;
  UINT64   Size;
  BOOLEAN  Fatal;
} RAM_CARVEOUT_EXCLUSION;

static inline BOOLEAN
RamCarveoutAppend (
  IN OUT RAM_CARVEOUT_RANGE  *Out,
  IN OUT UINTN               *Count,
  IN     UINTN               Capacity,
  IN     UINT64              Base,
  IN     UINT64              End
  )
{
  if (End <= Base) {
    return TRUE;
  }
  if (*Count >= Capacity) {
    return FALSE;
  }
  Out[*Count].Base = Base;
  Out[*Count].Size = End - Base;
  ++*Count;
  return TRUE;
}

/**
  Plan the reservations.  Returns FALSE on malformed input, Fatal overlap or capacity
  overflow; *OutCount is then 0.  On success Out[] is sorted and disjoint.
**/
static inline BOOLEAN
RamCarveoutPlan (
  IN  CONST RAM_CARVEOUT_RANGE      *Carveouts,
  IN  UINTN                         CarveoutCount,
  IN  UINT64                        RamBase,
  IN  UINT64                        RamSize,
  IN  CONST RAM_CARVEOUT_EXCLUSION  *Excluded,
  IN  UINTN                         ExcludedCount,
  OUT RAM_CARVEOUT_RANGE            *Out,
  IN  UINTN                         OutCapacity,
  OUT UINTN                         *OutCount
  )
{
  RAM_CARVEOUT_RANGE  Merged[RAM_CARVEOUT_MAX_INPUT];
  UINTN               MergedCount;
  UINT64              RamEnd;
  UINTN               Index;
  UINTN               Inner;

  if (OutCount == NULL) {
    return FALSE;
  }
  *OutCount = 0;
  if ((Carveouts == NULL && CarveoutCount != 0) || (Excluded == NULL && ExcludedCount != 0) ||
      Out == NULL || CarveoutCount > RAM_CARVEOUT_MAX_INPUT || RamSize == 0 ||
      RamBase > (~(UINT64)0) - RamSize ||
      (RamBase & (RAM_CARVEOUT_PAGE - 1)) != 0 || (RamSize & (RAM_CARVEOUT_PAGE - 1)) != 0) {
    return FALSE;
  }
  RamEnd = RamBase + RamSize;

  //
  // Clip, align, drop the kernel descriptor, then insertion-sort by base.
  //
  MergedCount = 0;
  for (Index = 0; Index < CarveoutCount; ++Index) {
    UINT64  Base = Carveouts[Index].Base;
    UINT64  Size = Carveouts[Index].Size;
    UINT64  End;
    UINT64  Start;
    UINT64  Stop;

    if (Size == 0 || Size > (~(UINT64)0) - RAM_CARVEOUT_PAGE ||
        Base > (~(UINT64)0) - RAM_CARVEOUT_PAGE - Size) {
      return FALSE;
    }
    End = Base + Size;
    if (Base <= RamBase && RamEnd <= End) {
      continue;
    }
    Start = Base & ~(RAM_CARVEOUT_PAGE - 1);
    Stop  = (End + RAM_CARVEOUT_PAGE - 1) & ~(RAM_CARVEOUT_PAGE - 1);
    if (Start < RamBase) {
      Start = RamBase;
    }
    if (Stop > RamEnd) {
      Stop = RamEnd;
    }
    if (Start >= Stop) {
      continue;
    }
    Inner = MergedCount++;
    while (Inner > 0 && Merged[Inner - 1].Base > Start) {
      Merged[Inner] = Merged[Inner - 1];
      --Inner;
    }
    Merged[Inner].Base = Start;
    Merged[Inner].Size = Stop - Start;
  }

  //
  // Merge overlapping, adjacent and <= RAM_CARVEOUT_MERGE_GAP separated ranges.
  //
  if (MergedCount > 1) {
    UINTN  Write = 0;
    for (Index = 1; Index < MergedCount; ++Index) {
      UINT64  LastEnd = Merged[Write].Base + Merged[Write].Size;
      UINT64  End     = Merged[Index].Base + Merged[Index].Size;
      if (Merged[Index].Base <= LastEnd + RAM_CARVEOUT_MERGE_GAP) {
        if (End > LastEnd) {
          Merged[Write].Size = End - Merged[Write].Base;
        }
      } else {
        Merged[++Write] = Merged[Index];
      }
    }
    MergedCount = Write + 1;
  }

  //
  // Fatal overlaps fail; owned ranges are subtracted (each may split a range).
  //
  for (Index = 0; Index < ExcludedCount; ++Index) {
    if (Excluded[Index].Size == 0 ||
        Excluded[Index].Base > (~(UINT64)0) - Excluded[Index].Size) {
      return FALSE;
    }
    if (!Excluded[Index].Fatal) {
      continue;
    }
    for (Inner = 0; Inner < MergedCount; ++Inner) {
      if (Excluded[Index].Base < Merged[Inner].Base + Merged[Inner].Size &&
          Merged[Inner].Base < Excluded[Index].Base + Excluded[Index].Size) {
        return FALSE;
      }
    }
  }

  for (Index = 0; Index < MergedCount; ++Index) {
    UINT64  Cursor = Merged[Index].Base;
    UINT64  End    = Merged[Index].Base + Merged[Index].Size;

    while (Cursor < End) {
      UINT64  Next = End;
      UINT64  Skip = Cursor;
      for (Inner = 0; Inner < ExcludedCount; ++Inner) {
        UINT64  ExBase = Excluded[Inner].Base;
        UINT64  ExEnd  = ExBase + Excluded[Inner].Size;
        if (Excluded[Inner].Fatal || ExEnd <= Cursor || ExBase >= End) {
          continue;
        }
        if (ExBase <= Cursor) {
          if (ExEnd > Skip) {
            Skip = ExEnd;            // Cursor lies inside an owned range
          }
        } else if (ExBase < Next) {
          Next = ExBase;             // nearest owned range ahead
        }
      }
      if (Skip > Cursor) {
        Cursor = Skip < End ? Skip : End;
        continue;
      }
      if (!RamCarveoutAppend (Out, OutCount, OutCapacity, Cursor, Next)) {
        *OutCount = 0;
        return FALSE;
      }
      Cursor = Next;
    }
  }
  return TRUE;
}

#endif
