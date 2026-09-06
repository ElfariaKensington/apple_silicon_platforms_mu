/* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <Uefi.h>
#define APPLE_ANS_RAMDISK_ONLY 1
#define DEBUG(x) ((VOID)0)
#define ByProtocol 1
#define OUT
#define OPTIONAL
typedef VOID *EFI_EVENT;
typedef struct { BOOLEAN MediaPresent,RemovableMedia; } EFI_BLOCK_IO_MEDIA;
typedef struct { EFI_BLOCK_IO_MEDIA *Media; } EFI_BLOCK_IO_PROTOCOL;
typedef struct { int unused; } EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
static int gEfiBlockIoProtocolGuid,gEfiSimpleFileSystemProtocolGuid;
static EFI_HANDLE handles[2]={(EFI_HANDLE)1,(EFI_HANDLE)2};
static EFI_EVENT mReadyToBootEvent;
static BOOLEAN mWindowsStarted;
static unsigned starts,connects;
static EFI_STATUS fatal_status;
static BOOLEAN missing;
static jmp_buf terminal;
static EFI_STATUS HandleProtocol(EFI_HANDLE h,VOID*g,VOID**p)
{
 static EFI_BLOCK_IO_MEDIA media={TRUE,FALSE};static EFI_BLOCK_IO_PROTOCOL bio={&media};
 static EFI_SIMPLE_FILE_SYSTEM_PROTOCOL fs;
 (VOID)h;*p=g==&gEfiBlockIoProtocolGuid?(VOID*)&bio:(VOID*)&fs;return EFI_SUCCESS;
}
static EFI_STATUS LocateHandleBuffer(int kind,VOID*g,VOID*registration,UINTN*count,EFI_HANDLE**p)
{(VOID)kind;(VOID)g;(VOID)registration;*count=2;*p=handles;return EFI_SUCCESS;}
static VOID ConnectController(EFI_HANDLE h,VOID*a,VOID*b,BOOLEAN recursive)
{(VOID)a;(VOID)b;assert(recursive && h==(EFI_HANDLE)2);connects++;}
static VOID CloseEvent(EFI_EVENT e) {(VOID)e;}
static struct test_services {EFI_STATUS(*HandleProtocol)(EFI_HANDLE,VOID*,VOID**);
 EFI_STATUS(*LocateHandleBuffer)(int,VOID*,VOID*,UINTN*,EFI_HANDLE**);
 VOID(*ConnectController)(EFI_HANDLE,VOID*,VOID*,BOOLEAN);VOID(*CloseEvent)(EFI_EVENT);
} services={HandleProtocol,LocateHandleBuffer,ConnectController,CloseEvent};
static struct test_services *gBS=&services;
static VOID FreePool(VOID*p) {(VOID)p;}
static VOID *DevicePathFromHandle(EFI_HANDLE h) {return h;}
static BOOLEAN AppleAnsPreloadedBootPath(VOID*p,BOOLEAN file)
{assert(!file);return !missing && p==(VOID*)2;}
static BOOLEAN HasWindowsFallbackLoader(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL*fs) {(VOID)fs;return TRUE;}
static EFI_STATUS StartWindowsFromHandle(EFI_HANDLE h) {assert(h==(EFI_HANDLE)2);starts++;return EFI_SUCCESS;}
static VOID NativeBootFailed(EFI_STATUS s) {fatal_status=s;longjmp(terminal,1);}
#include ANS_TEST_AUTOBOOT_FUNCTIONS
int main(void)
{
 assert(!TryStartWindowsFromHandle((EFI_HANDLE)1) && !starts);
 if(!setjmp(terminal)){WindowsAutoBootReadyToBoot(NULL,NULL);assert(0);}
 assert(starts==1 && connects==1 && fatal_status==EFI_SUCCESS); /* Even returned-success is terminal. */
 mWindowsStarted=FALSE;missing=TRUE;starts=connects=0;
 if(!setjmp(terminal)){WindowsAutoBootReadyToBoot(NULL,NULL);assert(0);}
 assert(!starts && !connects && fatal_status==EFI_NOT_FOUND);
 mWindowsStarted=TRUE;WindowsAutoBootReadyToBoot(NULL,NULL); /* Approved loader reentrant event. */
 puts("Actual native WindowsAutoBoot source/fallback policy tests PASS");return 0;
}
