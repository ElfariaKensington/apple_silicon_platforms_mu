/* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include <assert.h>
#include <stdio.h>
#include <Uefi.h>
#include <Protocol/DevicePath.h>
#define OUT
#define OPTIONAL
#define DEBUG(x) ((VOID)0)
#define EFI_UNSUPPORTED EFIERR(3)
#define EFI_IMAGE_MACHINE_TYPE_SUPPORTED(x) ((x)==0xaa64)
#define EFI_PAGES_TO_SIZE(x) ((x)*4096)
typedef struct {EFI_STATUS(*RegisterImage)(VOID*,UINT64,UINTN,VOID**);} EMU;
typedef struct {BOOLEAN Started;EFI_STATUS LoadImageStatus;UINT16 Machine;EMU*PeCoffEmu;
 UINT64 ImageBasePage;UINTN NumberOfPages;VOID*EntryPoint;
 struct {UINT16 ImageType;} ImageContext;EFI_DEVICE_PATH_PROTOCOL*LoadedImageDevicePath;
} LOADED_IMAGE_PRIVATE_DATA;
static LOADED_IMAGE_PRIVATE_DATA data;
static unsigned gated;
static EFI_STATUS gate_status;
static LOADED_IMAGE_PRIVATE_DATA *CoreLoadedImageInfo(EFI_HANDLE h) {return h?&data:NULL;}
static EFI_STATUS DxeStartImageGate(EFI_HANDLE h,UINT16 type,CONST EFI_DEVICE_PATH_PROTOCOL*p)
{assert(h==(EFI_HANDLE)1 && type==10 && p==data.LoadedImageDevicePath);gated++;return gate_status;}
#include ANS_TEST_START_BOUNDARY
int main(void)
{
 data.Machine=0xaa64;data.ImageContext.ImageType=10;data.LoadedImageDevicePath=(VOID*)0x1234;
 gate_status=EFI_SECURITY_VIOLATION;
 assert(CoreStartImage((EFI_HANDLE)1,NULL,NULL)==EFI_SECURITY_VIOLATION && gated==1);
 gate_status=EFI_SUCCESS;assert(CoreStartImage((EFI_HANDLE)1,NULL,NULL)==EFI_SUCCESS && gated==2);
 data.LoadImageStatus=EFI_INVALID_PARAMETER;
 assert(CoreStartImage((EFI_HANDLE)1,NULL,NULL)==EFI_INVALID_PARAMETER && gated==2);
 puts("Actual CoreStartImage pre-entry source gate boundary tests PASS");return 0;
}
