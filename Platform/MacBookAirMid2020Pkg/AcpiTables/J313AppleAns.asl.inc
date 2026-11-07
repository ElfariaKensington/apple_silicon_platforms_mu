// Generated from config/j313-ans.json; synthetic allocation, not physical ANS MMIO.
// Include in a profile that defines ANSN only after native ownership is armed.
Device (ANS0) {
    Name (_HID, "APPL0003")
    Name (_UID, Zero)
    Name (_CCA, One)
    Name (_CRS, ResourceTemplate () {
        QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable,
                     ReadWrite, 0, 0x0000000300010000, 0x0000000300013FFF, 0, 0x4000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 889 }
    })
    Method (_STA, 0) { If (ANSN) { Return (0x0F) } Return (Zero) }
}
