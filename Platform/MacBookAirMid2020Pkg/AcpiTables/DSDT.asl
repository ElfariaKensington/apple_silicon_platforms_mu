/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     DSDT.asl
 * 
 * Abstract:
 *     Differentiated System Description Table. This source file implements the DSDT table
 *     for the MacBook Air (Mid 2020) platform.
 * 
 * Environment:
 *     UEFI firmware/runtime services.
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
 **/
#include <IndustryStandard/Acpi65.h>

 DefinitionBlock("DSDT.aml", "DSDT", 0x02, "Apple", "J31x", 0x8103) {
    Scope(\_SB) {
        // Platform-wide _OSC: grant what the OS requests, notably CPPC (bit 5), CPPC
        // revision 2 (bit 6) and flexible CPPC register address spaces (bit 7) so FxPPM
        // uses the SystemMemory _CPC registers below.
        Method(_OSC, 4) {
            CreateDWordField(Arg3, 0x00, PCDW)
            If (Arg0 != ToUUID("0811b06e-4a27-44f9-8d60-3cbbc22e7b48")) {
                PCDW |= 0x04                          // unrecognized UUID
            }
            Return (Arg3)
        }
        // Generated from config/j313-apple-input.json.  Keep the firmware,
        // hypervisor, and Windows driver resource contracts byte-for-byte in
        // sync; never duplicate these machine-specific values in this DSDT.
        Include ("J313AppleInput.asl.inc")

        //
        // Cluster low power states. On T8101/T8103, there are only 2 clusters, the P and E core clusters,
        // so should be easier to track
        //
        Name (CLPI, Package() {
            0, // Version
            0, // Level Index
            1, // Count
            Package() { // Power Gating state for Cluster
            1, // Min residency (uS)
            1, // Wake latency (uS)
            1, // Flags
            1, // Arch Context Flags
            0, //Residency Counter Frequency
            0, // No Parent State
            0x00000000, // Integer Entry method (currently NULL, TODO actually add an entry method)
            ResourceTemplate() { // Null Residency Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            ResourceTemplate() { // Null Usage Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            "ClusterRetention"
            },
        })

        //
        // Per processor low power states.
        //
        Name(PLPI, Package() {
            0, // Version
            0, // Level Index
            2, // Count
            Package() { // WFI for CPU
            1, // Min residency (uS)
            1, // Wake latency (uS)
            1, // Flags
            0, // Arch Context Flags
            0, //Residency Counter Frequency
            0, // No parent state
            ResourceTemplate () {
                // Register Entry method
                Register (SystemMemory,
                0x00,               // Bit Width
                0x00,               // Bit Offset
                0x00,         // Address
                0x00,               // Access Size
                )
            },
            ResourceTemplate() { // Null Residency Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            ResourceTemplate() { // Null Usage Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            "WFI",
            },
            Package() { // Power Gating state for CPU
            1, // Min residency (uS)
            1, // Wake latency (uS)
            1, // Flags
            1, // Arch Context Flags
            0, //Residency Counter Frequency
            1, // Parent node can be in any state
            ResourceTemplate () {
                // Register Entry method
                Register (SystemMemory,
                0x00,               // Bit Width
                0x00,               // Bit Offset
                0x00000000,         // Address
                0x00,               // Access Size
                )
            },
            ResourceTemplate() { // Null Residency Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            ResourceTemplate() { // Null Usage Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            "CorePwrDn"
            },
        })

        Device(COM0) {
            Name(_HID, "APPL8900") // naming it APPL8900 since the Samsung based UART was used since the 8900
            Name(_UID, Zero)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                ResourceProducer,     // ResourceUsage
                PosDecode,            // Decode
                MinFixed,             // IsMinFixed
                MaxFixed,             // IsMaxFixed
                NonCacheable,         // Cacheable
                ReadWrite,            // ReadAndWrite
                0x0000000000000000,   // AddressGranularity - GRA
                //
                // Literals on purpose. Trim.py does substitute the PCD here, but it emits
                // 0x235200000ULL - and iasl understands neither the ULL suffix nor the
                // parenthesised addition that produced the maximum, so the descriptor came
                // out with Min > Max (ASL error 6051). This file is T8103-specific anyway;
                // the value is PcdAppleUartBase from T810XFamilyPkg.dsc.inc.
                //
                0x0000000235200000,   // AddressMinimum - MIN
                0x0000000235200FFF,   // AddressMaximum - MAX
                0x0000000000000000,   // AddressTranslation - TRA
                0x0000000000001000    // RangeLength - LEN
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1097 }            
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // The Type-C port not occupied by the m1n1 proxy. AppleUsbTypeCBringupDxe
        // leaves this DWC3 instance in xHCI host mode and its DARTs in bypass, so
        // Windows can bind the inbox USBXHCI driver directly to the standard xHCI
        // register window. AIC 857 is usb-drd1's level-high interrupt on T8103.
        //
        Device(XHC1) {
            // This DWC3 has no standard xHCI debug capability.  PNP0D10 selects the
            // DebuggerSafe install path and, with KD active, Windows preserves a
            // debugger-owned controller instead of creating the normal xHCI rings.
            // PNP0D15 selects the inbox no-standard-debug path used by the other M1 boards.
            Name(_HID, EISAID("PNP0D15"))
            Name(_UID, One)
            Name(_CCA, One)
            Name(_CRS, ResourceTemplate() {
                QWordMemory(
                    ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000502280000, // Min - usb-drd1 / xHCI
                    0x000000050237FFFF, // Max
                    0x0000000000000000, // Translation
                    0x0000000000100000  // Length (1 MB)
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 857 }
            })
            Method(_STA) { Return(0xF) }
        }

        //
        // Emulated PCIe root bridge (host: m1n1 hv_pci.c). ECAM is described by MCFG; this
        // node gives the OS the bus range, the MMIO window downstream BARs live in, and INTx
        // routing. Bare hex literals only (Trim.py/iasl PCD caveat, see COM0 above).
        //
        Device(PCI0) {
            Name(_HID, EISAID("PNP0A08"))   // PCIe host bridge
            Name(_CID, EISAID("PNP0A03"))   // ...also a legacy PCI host bridge
            Name(_SEG, Zero)
            Name(_BBN, Zero)
            Name(_CCA, One)                 // DMA is cache-coherent (guest RAM is coherent)
            Name(_UID, Zero)

            Name(_CRS, ResourceTemplate() {
                WordBusNumber(
                    ResourceProducer, MinFixed, MaxFixed, PosDecode,
                    0x0000,             // Granularity
                    0x0000,             // Min bus
                    0x0000,             // Max bus (single bus, 1 MB ECAM)
                    0x0000,             // Translation
                    0x0001              // Length (1 bus)
                )
                // MMIO window downstream BAR0 is assigned from (== PcdPciMmio64Base/Size).
                QWordMemory(
                    ResourceProducer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000400000000, // Min
                    0x000000041FFFFFFF, // Max
                    0x0000000000000000, // Translation
                    0x0000000020000000  // Length (512 MB)
                )
            })

            // INTx routing: device 0 INTA -> GSIV 64. A small vSPI that m1n1 injects DIRECTLY
            // into the vGIC (write a Group-1 LR + update_vi, same as the timer PPIs) with level
            // semantics tied to "unread CQ entries" - no AIC software line, so no AIC<->vGIC
            // namespace mixing. Level/ActiveLow is the INTx default for a GSIV _PRT entry.
            Name(_PRT, Package() {
                Package() { 0x0000FFFF, 0, Zero, 64 }   // dev0 INTA -> GSIV 64 (vSPI, direct inject)
            })

            // _OSC: hard NT requirement for a PNP0A08 root. Without it acpi.sys treats the
            // bridge as incomplete and never brings the segment up (MCFG present, ECAM never
            // read). Grant everything the OS asks for (no masking of CDW3).
            Method(_OSC, 4) {
                CreateDWordField(Arg3, 0x00, CDW1)   // status
                CreateDWordField(Arg3, 0x08, CDW3)   // control (left as requested)
                If (Arg0 == ToUUID("33db4d5b-1ff7-401c-9657-7441c03dd766")) {
                    // PCI Host Bridge UUID: grant all - CDW3 unchanged
                } Else {
                    CDW1 |= 0x04                      // unrecognized UUID
                }
                Return (Arg3)
            }

            Method(_STA) { Return(0xF) }
        }

        //
        // PCI Firmware Spec: the ECAM range from MCFG must be claimed as a motherboard
        // resource (PNP0C02) or pci.sys may ignore MCFG entirely - same silent symptom as a
        // missing _OSC. Reserve the 1 MB ECAM window at 0x690000000.
        //
        Device(RES0) {
            Name(_HID, EisaId("PNP0C02"))
            Name(_CRS, ResourceTemplate() {
                QWordMemory(
                    ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
                    0x0000000000000000, // Granularity
                    0x0000000690000000, // Min - ECAM base
                    0x00000006900FFFFF, // Max - ECAM base + 1 MB - 1
                    0x0000000000000000, // Translation
                    0x0000000000100000  // Length (1 MB)
                )
            })
            Method(_STA) { Return(0xF) }
        }

        //
        // T8101/T8103 *only* have 1 CPU die, ever, so everything in this node will comprise
        // most of the SoC.
        //
        Device(SOC) {
            Name(_HID, "ACPI0010") // all "processor containers" must have this HID
            Name(_UID, Zero) // unique identifier of the container

            //
            // E-core cluster, typically bootstrap core is here
            //
            Device(CLU0) {
                Name(_HID, "ACPI0010") // all "processor containers" must have this HID
                Name(_UID, 0x1) // unique identifier of the container

                Device(CPU0) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2064,        // Highest Performance
                        2064,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010000, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010008, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010010, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010018, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2064,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2064         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 0, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }

                Device(CPU1) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 1)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2064,        // Highest Performance
                        2064,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010040, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010048, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010050, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010058, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2064,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2064         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 0, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU2) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 2)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2064,        // Highest Performance
                        2064,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010080, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010088, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010090, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010098, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2064,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2064         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 0, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU3) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 3)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2064,        // Highest Performance
                        2064,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x3000100C0, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x3000100C8, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x3000100D0, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x3000100D8, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2064,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2064         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 0, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }

            //
            // P-core cluster.
            //
            Device(CLU1) {
                Name(_HID, "ACPI0010") // all "processor containers" must have this HID
                Name(_UID, 0x2) // unique identifier of the container
                Device(CPU4) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 4)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2988,        // Highest Performance
                        2988,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010100, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010108, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010110, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010118, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2988,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2988         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 1, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }

                Device(CPU5) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 5)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2988,        // Highest Performance
                        2988,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010140, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010148, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010150, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010158, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2988,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2988         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 1, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU6) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 6)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2988,        // Highest Performance
                        2988,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010180, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010188, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x300010190, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x300010198, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2988,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2988         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 1, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU7) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 7)
                    // ACPI CPPC (m1n1 hv_cppc.c page 0x300010000); performance unit = MHz.
                    Name(_CPC, Package() {
                        23,          // NumEntries
                        3,           // Revision
                        2988,        // Highest Performance
                        2988,        // Nominal Performance
                        600,         // Lowest Nonlinear Performance
                        600,         // Lowest Performance
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Guaranteed Performance Register
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x3000101C0, 3) }, // Desired Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Minimum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Maximum Performance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Performance Reduction Tolerance Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Time Window Register
                        0,           // Counter Wraparound Time (none)
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x3000101C8, 4) }, // Reference Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 64, 0, 0x3000101D0, 4) }, // Delivered Performance Counter
                        ResourceTemplate() { Register(SystemMemory, 32, 0, 0x3000101D8, 3) }, // Performance Limited Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // CPPC Enable Register
                        0,           // Autonomous Selection Enable
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Autonomous Activity Window Register
                        ResourceTemplate() { Register(SystemMemory, 0, 0, 0, 0) }, // Energy Performance Preference Register
                        2988,        // Reference Performance
                        600,         // Lowest Frequency (MHz)
                        2988         // Nominal Frequency (MHz)
                    })
                    // One P-state per cluster; the HV coordinates (max over the cluster).
                    Name(_PSD, Package() { Package() { 5, 0, 1, 0xFE, 4 } })
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }
        }
    }
 }
