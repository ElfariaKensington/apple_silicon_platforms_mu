# SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT

[Defines]
  DEFINE J316S_SETUP              = TRUE
  DEFINE J316S_SETUP_USB          = FALSE
  DEFINE J316S_SETUP_INPUT        = FALSE
  DEFINE J316S_SETUP_STORAGE      = FALSE
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
!if $(J316S_SETUP_STORAGE) == TRUE
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS -DJ316S_SETUP -DJ316S_SETUP_STORAGE
!else
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS -DJ316S_SETUP
!endif
!if $(J316S_SETUP_USB) == TRUE
!if $(J316S_SETUP_INPUT) == TRUE
  *_*_*_ASLPP_FLAGS = -DJ316S_SETUP_USB -DJ316S_SETUP_INPUT
!else
  *_*_*_ASLPP_FLAGS = -DJ316S_SETUP_USB
!endif
!endif

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
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemFamily|"MacBook Pro"
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|1728
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|1117
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|32
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
!if $(J316S_SETUP_STORAGE) == TRUE
  gArmTokenSpaceGuid.PcdPciBusMin|0
  gArmTokenSpaceGuid.PcdPciBusMax|0
  gArmTokenSpaceGuid.PcdPciMmio32Base|0xC0000000
  gArmTokenSpaceGuid.PcdPciMmio32Size|0
  gArmTokenSpaceGuid.PcdPciMmio64Base|0x400000000
  gArmTokenSpaceGuid.PcdPciMmio64Size|0x1000000
  gEfiMdePkgTokenSpaceGuid.PcdPciMmio32Translation|0
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0x690000000
  gEmbeddedTokenSpaceGuid.PcdDmaDeviceOffset|0
!endif

[PcdsDynamicDefault.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1728
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1117
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1728
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1117
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|25
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|80
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|25
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|80

[LibraryClasses.common]
  MemoryInitPeiLib|T810XFamilyPkg/Library/MemoryInitPeiLib/MemoryInitPeiLib.inf
  MsPlatformDevicesLib|T810XFamilyPkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf

[Components.common]
!if $(J316S_SETUP_STORAGE) == TRUE
  J316sSetupPkg/AcpiTables/J316sStorageTables.inf
!else
  J316sSetupPkg/AcpiTables/J316sDeviceTables.inf
!endif
  J316sSetupPkg/AcpiTables/J316sCpuTables.inf
  MacBookAirFamilyPkg/AcpiTables/DeviceFamilyAcpiTables.inf

!include AppleSiliconPkg/AppleSiliconPkg.dsc.inc
!include AppleSiliconPkg/FrontpageDsc.inc
