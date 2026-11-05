# SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT

[Defines]
  DEFINE J316S_SETUP              = TRUE
  PLATFORM_NAME                  = J316sSetup
  PLATFORM_GUID                  = 280951a5-55dc-4997-9ea1-8e76a1217424
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/J316sSetup-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = J316sSetupPkg/J316sSetup.fdf
  SECURE_BOOT_ENABLE             = FALSE
  AIC_BUILD                      = FALSE
  NETWORK_TLS_ENABLE             = TRUE
  J313_AGX_G2_PROFILE            = FALSE
  J313_AGX_ABI_ADMISSION_PROFILE  = FALSE
  J313_AGX_RESERVE_V2_PROFILE     = FALSE

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=6000
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS -DJ316S_SETUP

[PcdsPatchableInModule.common]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x850000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x1B0000000
  gArmTokenSpaceGuid.PcdFdBaseAddress|0
  gArmTokenSpaceGuid.PcdFvBaseAddress|0
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferAddress|0
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferSize|0

[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSocIdentifier|0x6000
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModel|"MacBook Pro (16-inch, M1 Pro, 2021)"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModelNumber|"MacBookPro18,1"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuIdentifier|"T6000"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuModel|"Apple M1 Pro"
  gAppleSiliconPkgTokenSpaceGuid.PcdCoreCount|10
  gAppleSiliconPkgTokenSpaceGuid.PcdClusterCount|3
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleUartBase|0x39B200000
  gAppleSiliconPkgTokenSpaceGuid.PcdInitializeRamdisk|TRUE
  gAppleSiliconPkgTokenSpaceGuid.PcdBootArgsPointer|0x854000000
  gAppleSiliconPkgTokenSpaceGuid.PcdAdtPointer|0x851000000
  gAppleSiliconPkgTokenSpaceGuid.PcdPreloadedRamdiskBase|0x900000000
  gAppleSiliconPkgTokenSpaceGuid.PcdPreloadedRamdiskMaxSize|0x80000000
  gAppleSiliconPkgTokenSpaceGuid.PcdLowMemoryWindowBase|0x100000
  gAppleSiliconPkgTokenSpaceGuid.PcdLowMemoryWindowSize|0x3FF00000
  gAppleSiliconPkgTokenSpaceGuid.PcdLowMemoryWindowBackingBase|0x8A0100000
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x5000000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x5100000000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|19
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|17
  gArmTokenSpaceGuid.PcdArmArchTimerHypIntrNum|20
  gArmTokenSpaceGuid.PcdArmArchTimerVirtIntrNum|18
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|40
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00006000
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE

[LibraryClasses.common]
  MemoryInitPeiLib|T810XFamilyPkg/Library/MemoryInitPeiLib/MemoryInitPeiLib.inf
  MsPlatformDevicesLib|T810XFamilyPkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf

[Components.common]
  J316sSetupPkg/AcpiTables/J316sDeviceTables.inf
  J316sSetupPkg/AcpiTables/J316sCpuTables.inf
  AppleSiliconPkg/Drivers/WindowsAutoBootDxe/WindowsAutoBootDxe.inf

!include MacBookAirFamilyPkg/MacBookAirFamily.dsc.inc
!include AppleSiliconPkg/AppleSiliconPkg.dsc.inc
!include AppleSiliconPkg/FrontpageDsc.inc
