# Offline J313 ExitBootServices gate

Source inspected before implementation: DXE CoreExitBootServices and
CoreTerminateMemoryMap, the Apple DxeCore component override, Air DSC/FDF,
AcpiPlatformDxe's PcdDeviceAcpiTableStorageFile selection, baseline DSDT and G2
ACPI storage, and the generated broker handoff ABI. CoreTerminateMemoryMap
only validates the current MapKey and construction rules; it does not commit
memory-map termination. The new gate runs after that successful validation and
before gMemoryMapTerminated and exit-event notification. A failed native gate
is terminal; it cannot forge a MapKey error or invite a retry after handoff.

The null library succeeds without MMIO. Apple sibling platforms bind null when
the ANS profile is absent; the Air's J313_ANS_NATIVE_PROFILE defaults FALSE.
The native library snapshots the synthetic broker header twice, writes one
nonzero sequence doorbell, then checks the complete generated receipt proof.
Failure stage/status and all eight receipt words remain in globals. It uses no
allocation, protocols, timer, physical ANS register, console, or reset.

`Tests/run.sh` runs the actual native and null C through host UEFI/IoLib shims
with ASan/UBSan, and extracts the actual CoreExitBootServices prefix through its
exit notification to test ordering, MapKey rejection/retry, and terminal gate
failure with the fallback. `Tests/iasl.sh` uses the standard EDK ASL trim and
preprocessing pipeline in an isolated container, with this worktree read-only
and only its dedicated output folder writable. It checks baseline and native
DSDT plus the existing unchanged G2 SSDT; it does not build firmware.

No usable native image is established. EL2 must implement the bounded,
synchronous trap, real ownership transition, and coherent receipt before this
library can run. Generated ACPI/profile selection alone cannot prove that
contract. Full firmware build/qualification is separately preregistered and
hardware requires explicit permission. Original GPU ACPI files remain unchanged.

## Native qualification boot-source restriction

`J313_ANS_NATIVE_PROFILE` is a qualification profile, not installed Windows native
boot. It now selects `WindowsAutoBootAnsNative.inf`; recovery still selects the
original auto-boot implementation. The native path connects/selects only a
RAMDisk whose device path matches the existing PCD-preloaded ASIRAMDK payload
range and volatile virtual-disk GUID. Missing media or any returned boot manager
is terminal at ReadyToBoot; no installed-disk fallback is attempted.

The stronger check lives in DxeCore's new `DxeStartImageGate`: before entering any
EFI application, the native library validates CoreLoadImage's already-owned full
LoadedImageDevicePath, including the RAMDisk node, optional partition, Windows
boot-manager filename, and final end-entire node. Internal PCI/NVMe/HD ancestry,
arbitrary firmware applications, memory-only loads, multi-instance paths and
unknown subsystem types are rejected before execution. Boot/runtime DXE driver
subsystems retain their original dispatch. The null implementation accepts all
images, preserving recovery behavior.

Only an accepted boot-manager handle is cached. Allocation failure or return from
CoreStartImage clears that handle before unload/reuse. ExitBootServices receives
its caller's ImageHandle and compares it with the cached handle before the ANS
handoff. This check performs no protocol lookup, device-path traversal, allocation
or preloaded-header access after successful MapKey validation.

Source inspected: UEFI2.10 chapter9 (Loaded Image Device Path copy), local
MdePkg DevicePath.h and MdeModulePkg RamDiskDxe/RamDiskProtocol.c (38-byte RAMDisk
node, inclusive end, parentNULL), CoreLoadImage/CoreStartImage, UefiBootManagerLib
BmBoot.c, BootRamdiskHelperDxe and the Air PCD layout. Preloaded header definitions
were moved unchanged into PreloadedRamdisk.h for a single shared ABI definition.
The existing recovery BOOTAA64.EFI was read from the already-owned signature-VM
media: ARM64 machine0xaa64, PE subsystem10 (EFI application), SHA256
`3dca6d137e7b61e231969d693d9fb86e590dea066931e686d40221f72d74ac8e`.
It is evidence for the accepted subsystem, not authorization to boot that image
on hardware. The live preloaded image still requires the launcher's preregistered
hash/manifest verification; path identity does not prove its filesystem contents.

Tests now exercise the actual source predicate, cached handle lifetime, native
auto-boot functions with internal/missing RAMDisk handles, and extracted actual
CoreStartImage pre-entry boundary. Unknown remaining hardware ordering: the
selected Windows boot manager must supply its admitted EFI handle at EBS; the
qualification candidate fails closed if it uses another handle. No assumption
that winload is separately dispatched through EFI StartImage is needed or granted.
A later installed-Windows/native-C boot requires a different explicitly approved
profile after storage qualification, not weakening this qualification guard.

Official loaded-image source:
https://uefi.org/specs/UEFI/2.10/09_Protocols_EFI_Loaded_Image.html
