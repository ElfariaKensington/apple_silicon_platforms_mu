// SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT

DefinitionBlock ("", "DSDT", 2, "APPLE ", "J316SPE ", 1)
{
    // Setup boots from memory. USB is optional and initialized by EL2.
    Scope (\_SB)
    {
#ifdef J316S_SETUP_WIFI
        Device (WIF0)
        {
            Name (_HID, "APPL0010")
            Name (_UID, Zero)
            Name (_CCA, One)
            Name (_CRS, ResourceTemplate ()
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0x410000000, 0x41007FFFF, 0, 0x80000)
            })
        }
#endif
        Device (CPU0) { Name (_HID, "ACPI0007") Name (_UID, 0) }
        Device (CPU1) { Name (_HID, "ACPI0007") Name (_UID, 1) }
        Device (CPU2) { Name (_HID, "ACPI0007") Name (_UID, 2) }
        Device (CPU3) { Name (_HID, "ACPI0007") Name (_UID, 3) }
        Device (CPU4) { Name (_HID, "ACPI0007") Name (_UID, 4) }
        Device (CPU5) { Name (_HID, "ACPI0007") Name (_UID, 5) }
        Device (CPU6) { Name (_HID, "ACPI0007") Name (_UID, 6) }
        Device (CPU7) { Name (_HID, "ACPI0007") Name (_UID, 7) }
        Device (CPU8) { Name (_HID, "ACPI0007") Name (_UID, 8) }
        Device (CPU9) { Name (_HID, "ACPI0007") Name (_UID, 9) }
#ifdef J316S_SETUP_USB
        Device (XHC1)
        {
            Name (_HID, EisaId ("PNP0D15"))
            Name (_UID, Zero)
            Name (_CCA, One)
            Name (_CRS, ResourceTemplate ()
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0x702280000, 0x70228BFFF, 0, 0xC000)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 858 }
            })
        }
        Device (XHC2)
        {
            Name (_HID, EisaId ("PNP0D15"))
            Name (_UID, One)
            Name (_CCA, One)
            Name (_CRS, ResourceTemplate ()
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0xB02280000, 0xB0228BFFF, 0, 0xC000)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 857 }
            })
        }
        Device (XHC3)
        {
            Name (_HID, EisaId ("PNP0D15"))
            Name (_UID, 2)
            Name (_CCA, One)
            Name (_CRS, ResourceTemplate ()
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0xF02280000, 0xF0228BFFF, 0, 0xC000)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 859 }
            })
        }
#endif
#ifdef J316S_SETUP_STORAGE
        Device (PCI0)
        {
            Name (_HID, EisaId ("PNP0A08"))
            Name (_CID, EisaId ("PNP0A03"))
            Name (_SEG, Zero)
            Name (_BBN, Zero)
            Name (_CCA, One)
            Name (_UID, Zero)
            Name (_CRS, ResourceTemplate ()
            {
                WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode,
                    0, 0, 0, 0, 1)
                QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0x400000000, 0x400FFFFFF, 0, 0x1000000)
            })
            Name (_PRT, Package () { Package () { 0xFFFF, 0, Zero, 64 } })
            Method (_OSC, 4)
            {
                CreateDWordField (Arg3, 0, CDW1)
                If (Arg0 != ToUUID ("33db4d5b-1ff7-401c-9657-7441c03dd766"))
                {
                    CDW1 |= 4
                }
                Return (Arg3)
            }
        }
        Device (RES0)
        {
            Name (_HID, EisaId ("PNP0C02"))
            Name (_CRS, ResourceTemplate ()
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0x690000000, 0x6900FFFFF, 0, 0x100000)
            })
        }
#endif
#ifdef J316S_SETUP_INPUT
        Device (AIP0)
        {
            Name (_HID, "APPL0001")
            Name (_UID, Zero)
            Name (_CRS, ResourceTemplate ()
            {
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0x39B10C000, 0x39B10FFFF, 0, 0x4000)
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0x39B028000, 0x39B02BFFF, 0, 0x4000)
                QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed,
                    NonCacheable, ReadWrite, 0, 0x2921F0000, 0x2921F3FFF, 0, 0x4000)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 865 }
            })
        }
#endif
    }
}
