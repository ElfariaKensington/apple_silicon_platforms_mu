// SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT

DefinitionBlock ("", "DSDT", 2, "APPLE ", "J316SPE ", 1)
{
    // Setup runs from memory. The namespace has no physical devices.
    Scope (\_SB)
    {
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
    }
}
