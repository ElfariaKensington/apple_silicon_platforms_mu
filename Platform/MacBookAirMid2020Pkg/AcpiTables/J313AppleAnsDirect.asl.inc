// ANS-DIRECT (investigation/ANS_DIRECT_PLAN.md): Windows owns the ANS2 NVMe runtime.
// Physical resources equal config/j313-ans.json physical_resources_expected_from_pinned_dt
// (controller, mailbox) and drivers/storage/appleans/common/ans_direct.h (root contract test).
// m1n1 identity-maps exactly these two ranges and routes physical IRQ 590 to GSIV 960 after
// the ExitBootServices hand-off. The ASC CPU page and SART stay EL2-private.
// Same _HID/_UID/_CCA as the broker variant: the persisted APPL0003 devnode is unchanged.
Device (ANS0) {
    Name (_HID, "APPL0003")
    Name (_UID, Zero)
    Name (_CCA, One)
    Name (_CRS, ResourceTemplate () {
        QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable,
                     ReadWrite, 0, 0x000000027BCC0000, 0x000000027BCFFFFF, 0, 0x40000)
        QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable,
                     ReadWrite, 0, 0x0000000277408000, 0x000000027740BFFF, 0, 0x4000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 960 }
    })
    Method (_STA, 0) { If (ANSN) { Return (0x0F) } Return (Zero) }
}
