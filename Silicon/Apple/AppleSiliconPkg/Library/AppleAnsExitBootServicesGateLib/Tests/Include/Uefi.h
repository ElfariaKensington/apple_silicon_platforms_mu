#ifndef ANS_TEST_UEFI_H
#define ANS_TEST_UEFI_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef char CHAR8;
typedef uint16_t CHAR16;
typedef struct {uint32_t Data1;uint16_t Data2,Data3;uint8_t Data4[8];} EFI_GUID;
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uintptr_t UINTN;
typedef UINTN EFI_STATUS;
typedef void VOID;
typedef void *EFI_HANDLE;
typedef bool BOOLEAN;
#define CONST const
#define STATIC static
#define IN
#define EFIAPI
#define TRUE true
#define FALSE false
#define MAX_UINT64 UINT64_MAX
#define EFI_SUCCESS 0
#define EFIERR(n) ((UINTN)(n) | ((UINTN)1 << (sizeof(UINTN)*8-1)))
#define EFI_INVALID_PARAMETER EFIERR(2)
#define EFI_SECURITY_VIOLATION EFIERR(26)
#define EFI_NOT_FOUND EFIERR(14)
#define EFI_NOT_READY EFIERR(6)
#define EFI_DEVICE_ERROR EFIERR(7)
#define EFI_OUT_OF_RESOURCES EFIERR(9)
#define EFI_INCOMPATIBLE_VERSION EFIERR(25)
#define EFI_ERROR(s) (((s) & ((UINTN)1 << (sizeof(UINTN)*8-1))) != 0)
#endif
