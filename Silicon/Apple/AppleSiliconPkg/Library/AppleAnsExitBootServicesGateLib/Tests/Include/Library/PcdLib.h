#include <Uefi.h>
extern UINT64 gTestPreloadBase;
extern UINT32 gTestPreloadSize;
#define PcdGet64(Name) gTestPreloadBase
#define PcdGet32(Name) gTestPreloadSize
